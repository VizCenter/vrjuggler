/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001, 2002
 *   by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef CPPUNIT_TESTRESULT_H
#define CPPUNIT_TESTRESULT_H

#include <vector>

#ifndef CPPUNIT_GUARDS_H
#include "Guards.h"
#endif

#ifndef CPPUNIT_TESTFAILURE_H
#include "TestFailure.h"
#endif


class CppUnitException;
class Test;


class TestResult
{
    REFERENCEOBJECT (TestResult)

public:
                                        TestResult  ();
    virtual                             ~TestResult ();

    virtual void                        addError       (Test *test, CppUnitException *e);
    virtual void                        addFailure     (Test *test, CppUnitException *e);
    virtual void                        startTest      (Test *test);
    virtual void                        endTest        (Test *test);
    virtual int                         runTests       ();
    virtual int                         testErrors     ();
    virtual int                         testFailures   ();
    virtual bool                        wasSuccessful  ();
    virtual bool                        shouldStop     ();
    virtual void                        stop           ();

    virtual std::vector<TestFailure *>& errors         ();
    virtual std::vector<TestFailure *>& failures       ();


    class SynchronizationObject
    {
    public:
                                SynchronizationObject  () {}
        virtual                 ~SynchronizationObject () {}

        virtual void            lock                   () {}
        virtual void            unlock                 () {}
    };

    class ExclusiveZone
    {
        SynchronizationObject   *m_syncObject;

    public:
                                ExclusiveZone (SynchronizationObject *syncObject) 
                                : m_syncObject (syncObject) 
                                { m_syncObject->lock (); }

                                ~ExclusiveZone () 
                                { m_syncObject->unlock (); }
    };

protected:
    virtual void                setSynchronizationObject (SynchronizationObject *syncObject);

    std::vector<TestFailure *>  m_errors;
    std::vector<TestFailure *>  m_failures;
    int                         m_runTests;
    bool                        m_stop;
    SynchronizationObject       *m_syncObject;

};



// Construct a TestResult
inline TestResult::TestResult ()
: m_syncObject (new SynchronizationObject ())
{ m_runTests = 0; m_stop = false; }


// Adds an error to the list of errors. The passed in exception
// caused the error
inline void TestResult::addError (Test *test, CppUnitException *e)
{ ExclusiveZone zone (m_syncObject); m_errors.push_back (new TestFailure (test, e)); }


// Adds a failure to the list of failures. The passed in exception
// caused the failure.
inline void TestResult::addFailure (Test *test, CppUnitException *e)
{ ExclusiveZone zone (m_syncObject); m_failures.push_back (new TestFailure (test, e)); }


// Informs the result that a test will be started.
inline void TestResult::startTest (Test *test)
{ ExclusiveZone zone (m_syncObject); m_runTests++; }

  
// Informs the result that a test was completed.
inline void TestResult::endTest (Test *test)
{ ExclusiveZone zone (m_syncObject); }


// Gets the number of run tests.
inline int TestResult::runTests ()
{ ExclusiveZone zone (m_syncObject); return m_runTests; }


// Gets the number of detected errors.
inline int TestResult::testErrors ()
{ ExclusiveZone zone (m_syncObject); return m_errors.size (); }


// Gets the number of detected failures.
inline int TestResult::testFailures ()
{ ExclusiveZone zone (m_syncObject); return m_failures.size (); }


// Returns whether the entire test was successful or not.
inline bool TestResult::wasSuccessful ()
{ ExclusiveZone zone (m_syncObject); return m_failures.size () == 0 && m_errors.size () == 0; }


// Returns a vector of the errors.
inline std::vector<TestFailure *>& TestResult::errors ()
{ ExclusiveZone zone (m_syncObject); return m_errors; }


// Returns a vector of the failures.
inline std::vector<TestFailure *>& TestResult::failures ()
{ ExclusiveZone zone (m_syncObject); return m_failures; }


// Returns whether testing should be stopped
inline bool TestResult::shouldStop ()
{ ExclusiveZone zone (m_syncObject); return m_stop; }


// Stop testing
inline void TestResult::stop ()
{ ExclusiveZone zone (m_syncObject); m_stop = true; }


// Accept a new synchronization object for protection of this instance
// TestResult assumes ownership of the object
inline void TestResult::setSynchronizationObject (SynchronizationObject *syncObject)
{ delete m_syncObject; m_syncObject = syncObject; }


#endif
