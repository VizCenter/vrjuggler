#ifndef _PLXTEST_SERIALIZABLE_TEST_H_
#define _PLXTEST_SERIALIZABLE_TEST_H_

#include <string>

#include <vpr/Util/ReturnStatus.h>

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#include <vpr/IO/SerializableObject.h>


namespace vprTest
{

class SerializableTest : public CppUnit::TestCase
{
public:
   class Class1 : public vpr::SerializableObject
   {
   public:
      Class1() : charVal(0), shortVal(0), longVal(0), longlongVal(0), floatVal(0), doubleVal(0)
      {;}

      virtual vpr::ReturnStatus writeObject(vpr::ObjectWriter* writer)
      {
         writer->writeUint8(charVal);
         writer->writeUint16(shortVal);
         writer->writeUint32(longVal);
         writer->writeUint64(longlongVal);
         writer->writeFloat(floatVal);
         writer->writeDouble(doubleVal);
         return vpr::ReturnStatus::Succeed;
      }

      virtual vpr::ReturnStatus readObject(vpr::ObjectReader* reader)
      {
         charVal = reader->readUint8();
         shortVal = reader->readUint16();
         longVal = reader->readUint32();
         longlongVal = reader->readUint64();
         floatVal = reader->readFloat();
         doubleVal = reader->readDouble();
         return vpr::ReturnStatus::Succeed;
      }

      bool operator==(Class1& r) const
      {
         return ( (charVal == r.charVal) &&
                  (shortVal == r.shortVal) &&
                  (longVal == r.longVal) &&
                  (longlongVal == r.longlongVal) &&
                  (floatVal == r.floatVal) &&
                  (doubleVal == r.doubleVal) );
      }

   public:
      vpr::Uint8  charVal;
      vpr::Uint16 shortVal;
      vpr::Uint32 longVal;
      vpr::Uint64 longlongVal;
      float       floatVal;
      double      doubleVal;
   };

   class Class2 : public vpr::SerializableObject
   {
   public:
      Class2() : mFlag(true)
      {;}

      virtual vpr::ReturnStatus writeObject(vpr::ObjectWriter* writer)
      {
         mObj1.writeObject(writer);
         mObj2.writeObject(writer);
         writer->writeBool(mFlag);
         return vpr::ReturnStatus::Succeed;
      }

      virtual vpr::ReturnStatus readObject(vpr::ObjectReader* reader)
      {
         mObj1.readObject(reader);
         mObj2.readObject(reader);
         mFlag = reader->readBool();
         return vpr::ReturnStatus::Succeed;
      }

      bool operator==(Class2& r)
      {
         return ( (mObj1 == r.mObj1) &&
                  (mObj2 == r.mObj2) &&
                  (mFlag == r.mFlag) );
      }

   public:
      Class1   mObj1;
      Class1   mObj2;
      bool     mFlag;
   };

public:
   SerializableTest( std::string name )
   : CppUnit::TestCase (name)
   {;}

   SerializableTest()
   : CppUnit::TestCase ()
   {;}

   virtual ~SerializableTest()
   {}

   virtual void setUp()
   {;}

   virtual void tearDown()
   {
   }

   void testReadWriteSimple();

   void testReadWriteNested();

//   void testReadWritePlxAddr();

//   void testReadWriteRouterInfo();

   static CppUnit::Test* suite();

   static CppUnit::Test* interactiveSuite()
   {
      CppUnit::TestSuite* test_suite = new CppUnit::TestSuite ("InteractiveSerializableTest");

      return test_suite;
   }

protected:

};

};

#endif
