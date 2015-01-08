 /***************************************************************************
                      keytest.c  -  Methods for Key manipulation
                             -------------------
    begin                : Fri Sep 26 2008
    copyright            : (C) 2008 by Markus Raab
    email                : elektra@markus-raab.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/


/**
 * @defgroup keytest Methods for Making Tests
 * @ingroup key
 * @brief Methods to do various tests on Keys
 *
 * To use them:
 * @code
#include <kdb.h>
 * @endcode
 *
 *
 */



#ifdef HAVE_KDBCONFIG_H
#include "kdbconfig.h"
#endif

#if DEBUG && defined(HAVE_STDIO_H)
#include <stdio.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "kdb.h"
#include "kdbprivate.h"
#include "kdbinternal.h"

/** \internal
 *
 * Clear sync flag of a key.
 *
 * @param key the key object to work with
 * @return -1 on null key
 * @return new flags for that key otherwise
 * @ingroup keytest
 *
 */
int keyClearSync (Key *key)
{
	if (!key) return -1;

	keyflag_t semiflag = KEY_FLAG_SYNC;

	semiflag=~semiflag;
	key->flags &= semiflag;

	return key->flags;
}



/**
 * Test if a key needs to be synced to backend storage.
 *
 * If any key modification took place the key will be flagged
 * so that kdbSet() knows which keys were modified
 * and which not.
 *
 * After keyNew() the flag will normally be set, but after kdbGet()
 * and kdbSet() the flag will be removed. When you modify the key
 * the flag will be set again.
 *
 * In your application you can make use of that flag to know
 * if you changed something in a key after a kdbGet() or kdbSet().
 *
 * @note Note that the sync status will be updated on any change,
 * including meta data.
 *
 * @see keyNew()
 * @param key the key object to work with
 * @return 1 if @p key was changed in memory, 0 otherwise
 * @return -1 on NULL pointer
 * @ingroup keytest
 */
int keyNeedSync(const Key *key)
{
	if (!key) return -1;

	return (key->flags & KEY_FLAG_SYNC) == KEY_FLAG_SYNC;
}


int keyIsSpec(const Key *key)
{
	if (!key) return -1;

	if (key->key) return keyNameIsSpec(key->key);
	else return 0;
}

int keyIsProc(const Key *key)
{
	if (!key) return -1;

	if (key->key) return keyNameIsProc(key->key);
	else return 0;
}


int keyIsDir(const Key *key)
{
	if (!key) return -1;

	if (key->key) return keyNameIsDir(key->key);
	else return 0;
}


/**
 * Check whether a key is under the @p system namespace or not
 *
 * @param key the key object to work with
 * @return 1 if key name begins with @p system, 0 otherwise
 * @return -1 on NULL pointer
 * @see keyIsUser(), keySetName(), keyName()
 * @ingroup keytest
 *
 */
int keyIsSystem(const Key *key)
{
	if (!key) return -1;

	if (key->key) return keyNameIsSystem(key->key);
	else return 0;
}




/**
 * Check whether a key is under the @p user namespace or not.
 *
 * @param key the key object to work with
 * @return 1 if key name begins with @p user, 0 otherwise
 * @return -1 on NULL pointer
 * @see keyIsSystem(), keySetName(), keyName()
 * @ingroup keytest
 *
 */
int keyIsUser(const Key *key)
{
	if (!key) return -1;

	if (key->key) return keyNameIsUser(key->key);
	else return 0;
}

/**
 * Check if the key check is below the key key or not.
 *
 * Example:
@verbatim
key user/sw/app
check user/sw/app/key
@endverbatim
 *
 * returns true because check is below key
 *
 * Example:
@verbatim
key user/sw/app
check user/sw/app/folder/key
@endverbatim
 *
 * returns also true because check is indirect below key
 *
 * @param key the key object to work with
 * @param check the key to find the relative position of
 * @return 1 if check is below key
 * @return 0 if it is not below or if it is the same key
 * @see keySetName(), keyGetName(), keyIsDirectBelow()
 * @ingroup keytest
 *
 */
int keyIsBelow(const Key *key, const Key *check)
{
	const char * keyname = 0;
	const char * checkname = 0;
	ssize_t keysize = 0;
	ssize_t checksize = 0;

	if (!key || !check) return -1;

	keyname = keyName(key);
	checkname = keyName(check);
	keysize = keyGetNameSize(key);
	checksize = keyGetNameSize(check);

	if (keysize > checksize + 1) return 0;
	if (strncmp (keyname, checkname, keysize - 1)) return 0;
	if (checkname[keysize - 1] != '/') return 0;
	return 1;
}

/**
 * Check if a key is below or same or not.
 *
 * @param key the key object to work with
 * @see keyIsBelow()
 */
int keyIsBelowOrSame (const Key *key, const Key *check)
{
	if (!key || !check) return -1;

	const char *name1 = keyName(key);
	const char *name2 = keyName(check);

	if (keyIsBelow (key, check)) return 1;
	else if (!strcmp (name1, name2)) return 1;
	return 0;
}


/**
 * Check if the key check is direct below the key key or not.
 *
@verbatim
Example:
key user/sw/app
check user/sw/app/key

returns true because check is below key

Example:
key user/sw/app
check user/sw/app/folder/key

does not return true, because there is only a indirect relation
@endverbatim
 *
 * @param key the key object to work with
 * @param check the key to find the relative position of
 * @return 1 if check is below key
 * @return 0 if it is not below or if it is the same key
 * @return -1 on null pointer
 * @see keyIsBelow(), keySetName(), keyGetName()
 * @ingroup keytest
 *
 */
int keyIsDirectBelow(const Key *key, const Key *check)
{
	if (!key || !check) return -1;

	if (!keyIsBelow(key, check)) return 0;


	const char * checkname = keyUnescapedName(check);
	ssize_t keysize = keyGetUnescapedNameSize(key);
	ssize_t checksize = keyGetUnescapedNameSize(check);
	if (strchr(checkname + keysize, '\0') == checkname + checksize - 1) return 1;

	return 0;
}


/**
 * Information about the relation in the hierarchy between
 * two keys.
 *
 * Unlike keyCmp() the number gives information
 * about hierarchical information.
 *
 *
 * - If the keys are the same 0 is returned.
 * So it is the key itself.
@verbatim
user/key
user/key
@endverbatim
 *
 *@code
keySetName (key, "user/key/folder");
keySetName (check, "user/key/folder");
succeed_if (keyRel (key, check) == 0, "should be same");
 *@endcode
 *
 * @note this relation can be checked with keyCmp() too.
 *
 *
 * - If the key is direct below the other one 1 is returned.
 * That means that, in terms of hierarchy, no other key is
 * between them - it is a direct child.
@verbatim
user/key/folder
user/key/folder/child
@endverbatim
 *
 *@code
keySetName (key, "user/key/folder");
keySetName (check, "user/key/folder/child");
succeed_if (keyRel (key, check) == 1, "should be direct below");
 *@endcode
 *
 *
 * - If the key is below the other one, but not directly 2 is returned.
 * This is also called grand-child.
@verbatim
user/key/folder
user/key/folder/any/depth/deeper/grand-child
@endverbatim
 *
 *
 *@code
keySetName (key, "user/key/folder");
keySetName (check, "user/key/folder/any/depth/deeper/grand-child");
succeed_if (keyRel (key, check) >= 2, "should be below (but not direct)");
succeed_if (keyRel (key, check) > 0, "should be below");
succeed_if (keyRel (key, check) >= 0, "should be the same or below");
 *@endcode
 *
 *
 * - If a invalid or null ptr key is passed, -1 is returned
 *
 *
 * - If the keys have no relations, but are not invalid, -2 is returned.
 *
 *
 * - If the keys are in the same hierarchy, a value smaller then -2 is returned.
 * It means that the key is not below.
@verbatim
user/key/myself
user/key/sibling
@endverbatim
 *
 * @code
keySetName (key, "user/key/folder");
keySetName (check, "user/notsame/folder");
succeed_if (keyRel (key, check) < -2, "key is not below, but same namespace");
 * @endcode
 *
 * @code
 * @endcode
 *
 *
 * TODO Below is an idea how it could be extended:
 * It could continue the search into the other direction
 * if any (grand-)parents are equal.
 *
 * - If the keys are direct below a key which is next to the key, -2 is returned.
 * This is also called nephew. (TODO not implemented)
 * @verbatim
user/key/myself
user/key/sibling
@endverbatim
 *
 * - If the keys are direct below a key which is next to the key, -2 is returned.
 * This is also called nephew. (TODO not implemented)
 * @verbatim
user/key/myself
user/key/sibling/nephew
@endverbatim
 *
 * - If the keys are below a key which is next to the key, -3 is returned.
 * This is also called grand-nephew. (TODO not implemented)
@verbatim
user/key/myself
user/key/sibling/any/depth/deeper/grand-nephew
@endverbatim
 *
 * The same holds true for the other direction, but with negative values.
 * For no relation INT_MIN is returned.
 *
 * @note to check if the keys are the same, you must use
 *       keyCmp() == 0!
 *       keyRel() does not give you the information if it did not
 *       find a relation or if it is the same key.
 *
 * @return dependend on the relation:
 *   2.. if below
 *   1.. if direct below
 *   0.. if the same
 *  -1.. on null or invalid keys
 *  -2.. if none of any other relation
 *  -3.. if same hierarchy (none of those below)
 *  -4.. if sibling (in same hierarchy)
 *  -5.. if nephew (in same hierarchy)
 *
 * @param key the key object to work with
 * @param check the second key object to check the relation with
 * @ingroup keytest
 */
int keyRel (const Key *key, const Key *check)
{
	if (!key || !check) return -1;
	if (!key->key || !check->key) return -1;

	if (!keyCmp(key,check)) return 0;
	if (keyIsDirectBelow(key, check)) return 1;
	if (keyIsBelow(key, check)) return 2;
	if (keyIsUser(key) && keyIsUser(check)) return -3;
	if (keyIsSystem(key) && keyIsSystem(check)) return -3;
	// if (keyIsSibling(key, check)) return -4;
	// if (keyIsNephew(key, check)) return -5;

	return -2;
}


/**
 * Check whether a key is inactive.
 *
 * In Elektra terminology a hierarchy of keys is inactive if
 * the rootkey's basename starts with '.'. So a key is
 * also inactive if it is below an inactive key.
 * For example, user/key/.hidden is inactive and so
 * is user/.hidden/below.
 *
 * Inactive keys should not have any meaning to applications,
 * they are only a convention reserved for users and
 * administrators. To automatically remove all inactive keys
 * for an application, consider to use the hidden plugin.
 *
 * @param key the key object to work with
 * @retval 1 if the key is inactive
 * @retval 0 if the key is active
 * @retval -1 on NULL pointer or when key has no name
 * @ingroup keytest
 *
 */
int keyIsInactive (const Key *key)
{
	if (!key) return -1;

	const char *p = keyName(key);
	if (!p) return -1;
	if (p[0] == '\0') return -1;

	size_t size=0;

	while (*(p=keyNameGetOneLevel(p+size,&size)))
	{
		if (size > 0)
		{
			if (p[0] == '.')
			{
				return 1;
			}
		}
	}

	return 0;
}



/**
 * Check if a key is binary type.
 *
 * The function checks if the key is a binary. Opposed to string values binary
 * values can have '\\0' inside the value and may not be terminated by a null
 * character. Their disadvantage is that you need to pass their size.
 *
 * Make sure to use this function and don't test the binary type another way to
 * ensure compatibility and to write less error prone programs.
 *
 * @return 1 if it is binary
 * @return 0 if it is not
 * @return -1 on NULL pointer
 * @see keyGetBinary(), keySetBinary()
 * @param key the key to check
 * @ingroup keytest
 */
int keyIsBinary(const Key *key)
{
	if (!key) return -1;

	return keyGetMeta(key, "binary") != 0;
}


/**
 * Check if a key is string type.
 *
 * String values are null terminated and are not allowed to have any '\\0' characters
 * inside the string.
 *
 * Make sure to use this function and don't test the string type another way to
 * ensure compatibility and to write less error prone programs.
 *
 * @return 1 if it is string
 * @return 0 if it is not
 * @return -1 on NULL pointer
 * @see keyGetString(), keySetString()
 * @param key the key to check
 * @ingroup keytest
 */
int keyIsString(const Key *key)
{
	if (!key) return -1;

	return keyGetMeta(key, "binary") == 0;
}





/**
 * @internal
 *
 * Compare 2 keys.
 *
 * The returned flags bit array has 1s (differ) or 0s (equal) for each key
 * meta info compared, that can be logically ORed using @c #keyswitch_t flags.
 * @link keyswitch_t::KEY_NAME KEY_NAME @endlink,
 * @link keyswitch_t::KEY_VALUE KEY_VALUE @endlink,
 * @link keyswitch_t::KEY_OWNER KEY_OWNER @endlink,
 * @link keyswitch_t::KEY_COMMENT KEY_COMMENT @endlink,
 * @link keyswitch_t::KEY_UID KEY_UID @endlink,
 * @link keyswitch_t::KEY_GID KEY_GID @endlink,
 * @link keyswitch_t::KEY_MODE KEY_MODE @endlink and
 *
 * @par A very simple example would be
 * @code
Key *key1, *key;
uint32_t changes;

// omited key1 and key2 initialization and manipulation

changes=keyCompare(key1,key2);

if (changes == 0) printf("key1 and key2 are identicall\n");

if (changes & KEY_VALUE)
	printf("key1 and key2 have different values\n");
 
if (changes & KEY_UID)
	printf("key1 and key2 have different UID\n");
 
 *
 * @endcode
 *
 * 
 * @par Example of very powerful specific Key lookup in a KeySet:
 * @code
Key *base = keyNew ("/sw/MyApp/something", KEY_END);
KDB *handle = kdbOpen(base);
KeySet *ks=ksNew(0, KS_END);
Key *current;
uint32_t match;
uint32_t interests;


kdbGet(handle, ks, base);

// we are interested only in key type and access permissions
interests=(KEY_TYPE | KEY_MODE);

ksRewind(ks);   // put cursor in the beginning
while ((curren=ksNext(ks))) {
	match=keyCompare(current,base);
	
	if ((~match & interests) == interests)
		printf("Key %s has same type and permissions of base key",keyName(current));

	// continue walking in the KeySet....
}

// now we want same name and/or value
interests=(KEY_NAME | KEY_VALUE);

// we don't really need ksRewind(), since previous loop achieved end of KeySet
ksRewind(ks);
while ((current=ksNext(ks))) {
	match=keyCompare(current,base);

	if ((~match & interests) == interests) {
		printf("Key %s has same name, value, and sync status
			of base key",keyName(current));
	}
	// continue walking in the KeySet....
}

ksDel(ks);
kdbClose (handle, base);
keyDel(base);
 * @endcode
 * 
 * @return a bit array pointing the differences
 * @param key1 first key
 * @param key2 second key
 * @see #keyswitch_t
 * @ingroup keytest
 */
keyswitch_t keyCompare(const Key *key1, const Key *key2)
{
	if (!key1 && !key2) return 0;
	if (!key1 || !key2) return KEY_NULL;

	keyswitch_t ret=0;
	ssize_t nsize1 = keyGetNameSize(key1);
	ssize_t nsize2 = keyGetNameSize(key2);
	const char *name1 = keyName(key1);
	const char *name2 = keyName(key2);
	const char *comment1 = keyComment(key1);
	const char *comment2 = keyComment(key2);
	const char *owner1 = keyOwner(key1);
	const char *owner2 = keyOwner(key2);
	const void *value1 = keyValue(key1);
	const void *value2 = keyValue(key2);
	ssize_t size1 = keyGetValueSize(key1);
	ssize_t size2 = keyGetValueSize(key2);

	// TODO: metadata not compared?

#ifndef WIN32
	if (keyGetUID(key1) != keyGetUID(key2))  ret|=KEY_UID;
	if (keyGetGID(key1) != keyGetGID(key2))  ret|=KEY_GID;
	if (keyGetMode(key1)!= keyGetMode(key2)) ret|=KEY_MODE;
#endif
	if (nsize1 != nsize2)              ret|=KEY_NAME;
	if (strcmp(name1, name2))          ret|=KEY_NAME;
	if (strcmp(comment1, comment2))    ret|=KEY_COMMENT;
	if (strcmp(owner1, owner2))        ret|=KEY_OWNER;
	if (size1 != size2)                ret|=KEY_VALUE;
	if (memcmp(value1, value2, size1)) ret|=KEY_VALUE;

	return ret;
}


