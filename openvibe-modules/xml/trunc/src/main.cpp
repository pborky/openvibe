#include "xml/IReader.h"
#include "xml/TReaderCallbackProxy.h"
#include "xml/IWriter.h"
#include "xml/TWriterCallbackProxy.h"

#include <boost/static_assert.hpp>
namespace XML
{
	// static (on compile time) test of types here
	BOOST_STATIC_ASSERT(sizeof(uint64)!=sizeof(uint32));
	BOOST_STATIC_ASSERT(sizeof(uint64)!=sizeof(uint16));
	BOOST_STATIC_ASSERT(sizeof(uint64)!=sizeof(uint8));
	BOOST_STATIC_ASSERT(sizeof(uint32)!=sizeof(uint16));
	BOOST_STATIC_ASSERT(sizeof(uint32)!=sizeof(uint8));
	BOOST_STATIC_ASSERT(sizeof(uint16)!=sizeof(uint8));
	
	BOOST_STATIC_ASSERT(sizeof(int64)!=sizeof(int32));
	BOOST_STATIC_ASSERT(sizeof(int64)!=sizeof(int16));
	BOOST_STATIC_ASSERT(sizeof(int64)!=sizeof(int8));
	BOOST_STATIC_ASSERT(sizeof(int32)!=sizeof(int16));
	BOOST_STATIC_ASSERT(sizeof(int32)!=sizeof(int8));
	BOOST_STATIC_ASSERT(sizeof(int16)!=sizeof(int8));
	
#ifndef XML_OS_Windows
	BOOST_STATIC_ASSERT(sizeof(float80)!=sizeof(float64));
	BOOST_STATIC_ASSERT(sizeof(float80)!=sizeof(float32));
#endif
	BOOST_STATIC_ASSERT(sizeof(float64)!=sizeof(float32));
}

