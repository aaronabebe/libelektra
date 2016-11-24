#include <gtest/gtest.h>

#include <model_pluginformat.hpp>

/**
 * TESTS for kdbrest::models::PluginFormat
 */

TEST (kdbrestModelsPluginformatTest, ConstructorCheck)
{

	try
	{
		kdbrest::model::PluginFormat pf ("xml:xmltool");
	}
	catch (kdbrest::exception::FileformatPluginException & e)
	{
		ASSERT_TRUE (false);
	}

	try
	{
		kdbrest::model::PluginFormat pf ("xml", "xmltool");
	}
	catch (kdbrest::exception::FileformatPluginException & e)
	{
		ASSERT_TRUE (false);
	}

	kdbrest::model::PluginFormat pf ("xml", "xmltool");
	try
	{
		kdbrest::model::PluginFormat pf2 (pf);
	}
	catch (kdbrest::exception::FileformatPluginException & e)
	{
		ASSERT_TRUE (false);
	}
}

TEST (kdbrestModelsPluginformatTest, ValueCheck)
{

	kdbrest::model::PluginFormat pf ("xml:xmltool");
	ASSERT_EQ (pf.getFileformat (), "xml");
	ASSERT_EQ (pf.getPluginname (), "xmltool");

	kdbrest::model::PluginFormat pf2 ("xml", "xmltool");
	ASSERT_EQ (pf2.getFileformat (), "xml");
	ASSERT_EQ (pf2.getPluginname (), "xmltool");

	kdbrest::model::PluginFormat pf3 (pf2);
	ASSERT_EQ (pf3.getFileformat (), "xml");
	ASSERT_EQ (pf3.getPluginname (), "xmltool");
	ASSERT_EQ (pf2.getFileformat (), pf3.getFileformat ());
	ASSERT_EQ (pf2.getPluginname (), pf3.getPluginname ());
}
