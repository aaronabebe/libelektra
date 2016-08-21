/**
 * @file
 *
 * @brief test suite for the crypto plugin.
 * Contains shared functions for all compile variants.
 *
 * @copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include "crypto.h"
#include "gpg.h"
#include <kdb.h>
#include <kdbinternal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tests_internal.h>
#include <tests_plugin.h>

#include "test_key.h"

#define TEST_KEY_ID "DDEBEF9EE2DC931701338212DAF635B17F230E8D"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static const char strVal[] = "abcde";
static const kdb_octet_t binVal[] = { 0x01, 0x02, 0x03, 0x04 };

static int isMarkedForEncryption (const Key * k)
{
	const Key * metaEncrypt = keyGetMeta (k, ELEKTRA_CRYPTO_META_ENCRYPT);
	if (metaEncrypt && strcmp (keyString (metaEncrypt), "1") == 0)
	{
		return 1;
	}
	return 0;
}

/**
 * @brief create a new KeySet holding sample data for encryption and decryption.
 */
static KeySet * newTestdataKeySet ()
{
	Key * kUnchanged1 = keyNew ("user/crypto/test/nochange", KEY_END);
	Key * kUnchanged2 = keyNew ("user/crypto/test/nochange2", KEY_END);
	Key * kNull = keyNew ("user/crypto/test/mynull", KEY_END);
	Key * kString = keyNew ("user/crypto/test/mystring", KEY_END);
	Key * kBin = keyNew ("user/crypto/test/mybin", KEY_END);

	keySetString (kUnchanged1, strVal);

	keySetString (kUnchanged2, strVal);
	keySetMeta (kUnchanged2, ELEKTRA_CRYPTO_META_ENCRYPT, "0");

	keySetBinary (kNull, 0, 0);
	keySetMeta (kNull, ELEKTRA_CRYPTO_META_ENCRYPT, "1");

	keySetString (kString, strVal);
	keySetMeta (kString, ELEKTRA_CRYPTO_META_ENCRYPT, "1");

	keySetBinary (kBin, binVal, sizeof (binVal));
	keySetMeta (kBin, ELEKTRA_CRYPTO_META_ENCRYPT, "1");

	return ksNew (5, kUnchanged1, kUnchanged2, kNull, kString, kBin, KS_END);
}

static inline void setPluginShutdown (KeySet * config)
{
	ksAppendKey (config, keyNew (ELEKTRA_CRYPTO_PARAM_SHUTDOWN, KEY_VALUE, "1", 0));
}

static KeySet * newPluginConfiguration ()
{
	return ksNew (2, keyNew (ELEKTRA_CRYPTO_PARAM_GPG_KEY, KEY_VALUE, TEST_KEY_ID, KEY_END),
		      keyNew (ELEKTRA_CRYPTO_PARAM_GPG_UNIT_TEST, KEY_VALUE, "1", KEY_END), KS_END);
}

static void test_init (const char * pluginName)
{
	Plugin * plugin = NULL;
	Key * parentKey = keyNew ("system", KEY_END);
	KeySet * modules = ksNew (0, KS_END);
	KeySet * configKs = newPluginConfiguration ();
	elektraModulesInit (modules, 0);

	plugin = elektraPluginOpen (pluginName, modules, configKs, 0);
	succeed_if (plugin != 0, "failed to open the plugin");
	if (plugin)
	{
		succeed_if (!strcmp (plugin->name, pluginName), "got wrong name");

		KeySet * config = elektraPluginGetConfig (plugin);
		succeed_if (config != 0, "there should be a config");

		succeed_if (plugin->kdbOpen != 0, "no open pointer");
		succeed_if (plugin->kdbClose != 0, "no close pointer");
		succeed_if (plugin->kdbGet != 0, "no get pointer");
		succeed_if (plugin->kdbSet != 0, "no set pointer");
		succeed_if (plugin->kdbError != 0, "no error pointer");

		// try re-opening the plugin
		succeed_if (plugin->kdbClose (plugin, parentKey) == 1, "kdb close failed");
		succeed_if (plugin->kdbOpen (plugin, parentKey) == 1, "re-opening the plugin failed");
		succeed_if (plugin->kdbClose (plugin, parentKey) == 1, "kdb close failed");

		elektraPluginClose (plugin, 0);
	}

	elektraModulesClose (modules, 0);
	ksDel (modules);
	keyDel (parentKey);
}

static void test_crypto_operations (const char * pluginName)
{
	Plugin * plugin = NULL;
	Key * parentKey = keyNew ("system", KEY_END);
	KeySet * modules = ksNew (0, KS_END);
	KeySet * config = newPluginConfiguration ();

	setPluginShutdown (config);

	elektraModulesInit (modules, 0);

	plugin = elektraPluginOpen (pluginName, modules, config, 0);
	if (plugin)
	{
		Key * k;
		KeySet * data = newTestdataKeySet ();
		KeySet * original = ksDup (data);

		// run checkconf to generate the master password

		/*
		 * TODO call the checkconf function
		 * succeed_if (plugin->getSymbol("checkconf")->(parentKey, config), "checkconf failed");
		*/

		// test encryption with kdb set
		succeed_if (plugin->kdbSet (plugin, data, parentKey) == 1, "kdb set failed");

		// verify key set
		ksRewind (data);
		while ((k = ksNext (data)) != 0)
		{
			if (isMarkedForEncryption (k))
			{
				succeed_if (keyIsBinary (k), "Key value is not binary although it should have been encrypted");
				succeed_if (keyGetValueSize (k) > 0, "NULL Key must have encrypted meta-data and can not have length 0");
				succeed_if (memcmp (keyValue (k), binVal, MIN (keyGetValueSize (k), (ssize_t)sizeof (binVal))),
					    "encryption failed");
				succeed_if (memcmp (keyValue (k), strVal, MIN (keyGetValueSize (k), (ssize_t)sizeof (strVal))),
					    "encryption failed");
			}
			else
			{
				succeed_if (!strcmp (keyString (k), strVal), "Key value changed without being marked for encryption");
			}
		}

		// test decryption with kdb get
		succeed_if (plugin->kdbGet (plugin, data, parentKey) == 1, "kdb get failed");
		compare_keyset (data, original);

		ksDel (original);
		ksDel (data);
		elektraPluginClose (plugin, 0);
	}

	elektraModulesClose (modules, 0);
	ksDel (modules);
	keyDel (parentKey);
}

static void test_gpg (void)
{
	// Plugin configuration
	KeySet * conf = newPluginConfiguration ();
	Key * errorKey = keyNew (0);

	// install the gpg key
	char * argv[] = { "", "-a", "--import", NULL };
	const size_t argc = 4;
	Key * msg = keyNew (0);
	keySetBinary (msg, test_key_asc, test_key_asc_len);

	succeed_if (elektraCryptoGpgCall (conf, errorKey, msg, argv, argc) == 1, "failed to install the GPG test key");

	keyDel (msg);
	keyDel (errorKey);
	ksDel (conf);
}
