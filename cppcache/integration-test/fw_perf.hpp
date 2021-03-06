#pragma once

#ifndef GEODE_INTEGRATION_TEST_FW_PERF_H_
#define GEODE_INTEGRATION_TEST_FW_PERF_H_

/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**

fw_perf.hpp provides framework macros for measuring performance,
archiving the results, and comparing against previous archives.

Steps for a performance suite:

perf::PerfSuite suite( "SuiteName" );

const char* name = "my operation type/test description";
perf::TimeStamp starttime;
... perform n operations.
perf::TimeStamp stoptime;
suite.addRecord( name, opcount, starttime, stoptime );

suite.save( );    // will save current results to <suite>_results.<hostname>
suite.compare( ); // will compare against the file named
<suite>_baseline.<hostname>

If no baseline file for the host is available, then an error will occur,
recommending
that the results be analyzed for acceptability, and checked in as the hosts
baseline.

If a baseline is found, a comparison report is generated, if the deviation is
beyond
established limits, a TestException will be thrown.

*/

#include <string>
#include <map>

#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>

#include <ace/Task.h>
#include <ace/Condition_T.h>
#include <ace/Thread_Mutex.h>
//#include "Name_Handler.h"

namespace perf {

class Semaphore {
 private:
  ACE_Thread_Mutex m_mutex;
  ACE_Condition<ACE_Thread_Mutex> m_cond;
  volatile int m_count;

 public:
  Semaphore() = delete;
  explicit Semaphore(int count);
  ~Semaphore();
  Semaphore(const Semaphore& other) = delete;
  Semaphore& operator=(const Semaphore& other) = delete;

  void acquire(int t = 1);
  void release(int t = 1);
};

class TimeStamp {
 private:
  int64_t m_msec;

 public:
  TimeStamp();
  TimeStamp(const TimeStamp& other);
  explicit TimeStamp(int64_t msec);
  TimeStamp& operator=(const TimeStamp& other);

  ~TimeStamp();

  int64_t msec() const;
  void msec(int64_t t);
};

class Record {
 private:
  std::string m_testName;
  int64_t m_operations;
  TimeStamp m_startTime;
  TimeStamp m_stopTime;

 public:
  Record(std::string testName, int64_t ops, const TimeStamp& start,
         const TimeStamp& stop);

  Record();

  Record(const Record& other);

  Record& operator=(const Record& other);

  void write(apache::geode::client::DataOutput& output);

  void read(apache::geode::client::DataInput& input);

  int elapsed();
  int perSec();
  std::string asString();

  ~Record();
};

class PerfSuite {
 private:
  std::string m_suiteName;

 public:
  explicit PerfSuite(const char* suiteName);

  void addRecord(std::string testName, int64_t ops, const TimeStamp& start,
                 const TimeStamp& stop);

  /** create a file in cwd, named "<suite>_results.<host>" */
  void save();

  /** load data saved in $ENV{'baselines'} named "<suite>_baseline.<host>"
   *  A non-favorable comparison will throw an TestException.
   */
  void compare();
};

class Thread;

class ThreadLauncher {
 private:
  int m_thrCount;
  Semaphore m_initSemaphore;
  Semaphore m_startSemaphore;
  Semaphore m_stopSemaphore;
  Semaphore m_cleanSemaphore;
  Semaphore m_termSemaphore;
  TimeStamp* m_startTime;
  TimeStamp* m_stopTime;
  Thread& m_threadDef;

 public:
  ThreadLauncher(int thrCount, Thread& thr);

  void go();

  ~ThreadLauncher();

  Semaphore& initSemaphore() { return m_initSemaphore; }

  Semaphore& startSemaphore() { return m_startSemaphore; }

  Semaphore& stopSemaphore() { return m_stopSemaphore; }

  Semaphore& cleanSemaphore() { return m_cleanSemaphore; }

  Semaphore& termSemaphore() { return m_termSemaphore; }

  TimeStamp startTime() { return *m_startTime; }

  TimeStamp stopTime() { return *m_stopTime; }

 private:
  ThreadLauncher& operator=(const ThreadLauncher& other);
  ThreadLauncher(const ThreadLauncher& other);
};

class Thread : public ACE_Task_Base {
 private:
  ThreadLauncher* m_launcher;
  bool m_used;

 public:
  Thread() : ACE_Task_Base(), m_launcher(nullptr), m_used(false) {}

  // Unhide function to prevent SunPro Warnings
  using ACE_Shared_Object::init;
  void init(ThreadLauncher* l) {
    ASSERT(!m_used, "Cannot reliably reuse Thread.");
    m_launcher = l;
  }

  ~Thread() noexcept override = default;

  /** called before measurement begins. override to do per thread setup. */
  virtual void setup() {}

  /** run during measurement */
  virtual void perftask() = 0;

  /** called after measurement to clean up what might have been setup in setup..
   */
  virtual void cleanup() {}

  int svc() override;
};

}  // namespace perf

#endif  // GEODE_INTEGRATION_TEST_FW_PERF_H_
