#ifndef _VPRTEST_SOCKET_TEST_H
#define _VPRTEST_SOCKET_TEST_H

#include <string>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/ThreadTestCase.h>
#include <cppunit/TestSuite.h>

#include <vpr/IO/Socket/SocketStream.h>
#include <vpr/Sync/Mutex.h>
#include <vpr/Sync/CondVar.h>


namespace {

struct _thread_args {
   vpr::SocketStream* mSock;
};




typedef struct _thread_args thread_args_t;

}

namespace vprTest
{

class SocketTest : public CppUnit::ThreadTestCase
{
public:
   SocketTest( )
      : CppUnit::ThreadTestCase (),
        testSendRecv_buffer( "we're sending DATA!!!" )
   {
      init();
   }

   SocketTest(std::string name)
      : CppUnit::ThreadTestCase (name),
        testSendRecv_buffer( "we're sending DATA!!!" )
   {
      init();
   }

   virtual ~SocketTest()
   {
   }

   // =========================================================================
   // open-close * n test
   // start two threads, cli/serv... both opening and closing many many times.
   // =========================================================================
   void testOpenCloseOpen_connector( void* data );
   void testOpenCloseOpen_acceptor( void* data );
   void testOpenCloseOpen();
   // =========================================================================


   // =========================================================================
   // send-recv test
   // same as open/close * n test, but do send/recv of data.  do many times.
   // =========================================================================
   const std::string testSendRecv_buffer;
   void testSendRecv_connector( void* data );
   void testSendRecv_acceptor( void* data );
   void testSendRecv();
   // =========================================================================


   // =========================================================================
   // open-bind-close test
   // open, then bind, then close, fail if any of them fail...
   // =========================================================================
   void testOpenClose();

   // =========================================================================
   // bind again should fail test
   // open, then bind, then bind again.  this should fail.
   // =========================================================================
   void bindAgainFailTest();

   // =========================================================================
   // same-address-open-bind-close test
   // open, then bind, then close, repeat many times on the same address..
   // =========================================================================
   void sameAddressOpenBindCloseTest();

   // =========================================================================
   // same-address-open-bind-destruct test
   // open, then bind, then delete it, repeat many times on the same address..
   // =========================================================================
   void sameAddressOpenBindDestructTest();

   // =========================================================================
   // different-address-open-bind-close test
   // - Try to open, then close, then bind, repeat*n for a range of ports
   // =========================================================================
   void differentAddressOpenBindCloseTest();

   // =========================================================================
   // reuse address simple test
   // try to reuse the address... broken...
   // =========================================================================
   void reuseAddrSimpleTest();

   // =========================================================================
   // reuse address client/server test
   // try to reuse the address with serv/cli threads...
   // =========================================================================
   void reuseAddrTest_connector( void* data );
   void reuseAddrTest_acceptor( void* data );
   void reuseAddrTest();

   // =========================================================================
   // Blocking/Nonblocking test
   // does this work?  what's it do?
   // =========================================================================
   void testBlocking_connector(void* arg);
   void testBlocking_acceptor(void* arg);
   void testBlocking();


   void testTcpConnection();

   //Function for master server thread
   void serverFunc(void* arg);

   //function for creating client threads
   void clientFunc(void* arg);

   //function for creating slave server thread, the server sends message to client, gets the client's
   //echo, and then compares the two strings.
   void sServerFunc(void* arg);

   // =========================================================================
   // readn() test
   // =========================================================================
   void testReadnClient(void* arg);
   void testReadn();
   // =========================================================================

   // =========================================================================
   // Test for isConnected().  In this test, two threads are spawned, one for
   // the acceptor, and one for the connector.  The connector connects, and
   // then informs the acceptor when it closes its side of the socket.  The
   // acceptor and connector use isConnected() to verify the state of the
   // connection at key times.
   // =========================================================================
   void testIsConnected(void);
   void testIsConnected_acceptor(void* arg);
   void testIsConnected_connector(void* arg);
   // =========================================================================

   static CppUnit::Test* suite();

protected:
   void init ()
   {
      mOpenServerSuccess=0;
      mNumSServer=2;
      mNumClient=2;
      mFinishFlag=false;
      mBlockingFlag=true;
      mStartFlag=false;
      mReadnFlag=false;
   }

   vpr::Mutex     mItemProtectionMutex;         // Protect an exclusive item
   vpr::Mutex     mFlagProtectionMutex;         // Protect an flags using in blocking test
   bool           mFinishFlag;
   bool           mBlockingFlag;
   bool           mStartFlag;
   bool           mReadnFlag;
   long           mOpenServerSuccess;
   long           mNumSServer;
   long           mNumClient;
   long           mClientCounter;
   long           mServerCheck;

   enum State {
      ACCEPTOR_READY,
      ACCEPTOR_TESTED,
      NOT_READY,
      CONNECTOR_CLOSED,
      DATA_SENT,
      DONE_READING
   };

   State           mState;         // State variable
   vpr::CondVar    mCondVar;       // Condition variable
   vpr::Uint16     mAcceptorPort;
};

}

#endif
