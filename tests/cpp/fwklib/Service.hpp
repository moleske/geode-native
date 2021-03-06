#pragma once

#ifndef GEODE_FWKLIB_SERVICE_H_
#define GEODE_FWKLIB_SERVICE_H_

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

#include <atomic>
#include <fwklib/FwkLog.hpp>
#include <string>

#include <ace/Condition_T.h>
#include <ace/Task.h>

#include <geode/internal/geode_base.hpp>

namespace apache {
namespace geode {
namespace client {
namespace testframework {

class SharedTaskObject {
 public:
  SharedTaskObject() {}

  virtual ~SharedTaskObject() {}

  virtual void initialize() = 0;
  virtual void finalize() = 0;
};

class ServiceTask {
 protected:
  volatile bool* m_run;
  SharedTaskObject* m_shared;

 public:
  explicit ServiceTask(SharedTaskObject* shared)
      : m_run(nullptr), m_shared(shared) {}

  virtual ~ServiceTask() {}

  void setRunFlag(volatile bool* run) { m_run = run; }
  virtual int32_t doTask() = 0;
  virtual void initialize() = 0;
  virtual void finalize() = 0;
};

class Service : public ACE_Task_Base {
 private:
  uint32_t m_ThreadCount;
  volatile bool m_run;
  std::atomic<uint32_t> m_busy;
  ACE_Thread_Mutex m_Mutex;
  ACE_DLList<ServiceTask> m_TaskQueue;

  int32_t svc() override;

  inline void putQ(ServiceTask* task, uint32_t cnt = 1) {
    ACE_Guard<ACE_Thread_Mutex> guard(m_Mutex);
    m_busy += cnt;
    for (uint32_t i = 0; i < cnt; i++) m_TaskQueue.insert_tail(task);
  }

  inline ServiceTask* getQ() {
    ACE_Guard<ACE_Thread_Mutex> guard(m_Mutex);
    return m_TaskQueue.delete_head();
  }

 public:
  explicit Service(int32_t threadCnt);

  ~Service() override { stopThreads(); }

  int32_t runThreaded(ServiceTask* task, uint32_t threads);

  inline uint32_t getBusyCount() { return m_busy; }
  inline uint32_t getIdleCount() { return m_ThreadCount - m_busy; }

  inline void stopThreads() {
    m_run = false;
    wait();
  }
};

template <class T>
class SafeQueue {
  ACE_Thread_Mutex m_mutex;
  ACE_DLList<T> m_queue;
  ACE_Condition<ACE_Thread_Mutex> m_cond;

 public:
  SafeQueue() : m_mutex(), m_cond(m_mutex) {}
  ~SafeQueue() {}

  void enqueue(T* val) {
    ACE_Guard<ACE_Thread_Mutex> guard(m_mutex);
    m_queue.insert_tail(val);
    m_cond.signal();
  }

  T* dequeue() {
    ACE_Guard<ACE_Thread_Mutex> guard(m_mutex);
    if (m_queue.size() == 0) {
      ACE_Time_Value until(2);
      until += ACE_OS::gettimeofday();
      auto res = m_cond.wait(&until);
      if (res == -1) return nullptr;
    }
    return m_queue.delete_head();
  }

  bool isEmpty() {
    ACE_Guard<ACE_Thread_Mutex> guard(m_mutex);
    return m_queue.isEmpty();
  }

  uint32_t size() {
    ACE_Guard<ACE_Thread_Mutex> guard(m_mutex);
    return static_cast<uint32_t>(m_queue.size());
  }
};

class IPCMessage {
 protected:
  std::string m_msg;

 public:
  IPCMessage() {}
  explicit IPCMessage(std::string content) { m_msg = content; }

  virtual ~IPCMessage() {}

  std::string& what() { return m_msg; }

  uint32_t length() { return static_cast<uint32_t>(m_msg.size()); }

  const char* getContent() { return m_msg.c_str(); }

  void setMessage(std::string& content) { m_msg = content; }

  virtual void clear() { m_msg.clear(); }
};

}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FWKLIB_SERVICE_H_
