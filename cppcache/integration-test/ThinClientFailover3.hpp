#pragma once

#ifndef GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER3_H_
#define GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER3_H_

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

#include "fw_dunit.hpp"
#include <ace/OS.h>
#include <ace/High_Res_Timer.h>
#include <string>
#include <vector>

#define ROOT_NAME "ThinClientFailover3"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

namespace {

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;

bool isLocalServer = false;
const char* endPoints = CacheHelper::getTcrEndpoints(isLocalServer, 3);
CacheHelper* cacheHelper = nullptr;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2
static bool isLocator = false;
// static int numberOfLocators = 0;
const char* locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, 1);
#include "LocatorHelper.hpp"
void initClient(const bool isthinClient) {
  if (cacheHelper == nullptr) {
    cacheHelper = new CacheHelper(isthinClient);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}
void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

CacheHelper* getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void _verifyEntry(const char* name, const char* key, const char* val,
                  bool noKey) {
  // Verify key and value exist in this region, in this process.
  const char* value = val ? val : "";
  char* buf =
      reinterpret_cast<char*>(malloc(1024 + strlen(key) + strlen(value)));
  ASSERT(buf, "Unable to malloc buffer for logging.");
  if (noKey) {
    sprintf(buf, "Verify key %s does not exist in region %s", key, name);
  } else if (!val) {
    sprintf(buf, "Verify value for key %s does not exist in region %s", key,
            name);
  } else {
    sprintf(buf, "Verify value for key %s is: %s in region %s", key, value,
            name);
  }
  LOG(buf);
  free(buf);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto keyPtr = CacheableKey::create(key);

  if (noKey == false) {  // need to find the key!
    ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  uint32_t SLEEP = 10;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; i--) {
    if (noKey) {
      if (regPtr->containsKey(keyPtr)) {
        containsKeyCnt++;
      } else {
        break;
      }
      ASSERT(containsKeyCnt < MAX, "Key found in region.");
    }
    if (val == nullptr) {
      if (regPtr->containsValueForKey(keyPtr)) {
        containsValueCnt++;
      } else {
        break;
      }
      ASSERT(containsValueCnt < MAX, "Value found in region.");
    }

    if (val != nullptr) {
      auto checkPtr =
          std::dynamic_pointer_cast<CacheableString>(regPtr->get(keyPtr));

      ASSERT(checkPtr != nullptr, "Value Ptr should not be null.");
      char buf[1024];
      sprintf(buf, "In verify loop, get returned %s for key %s",
              checkPtr->value().c_str(), key);
      LOG(buf);
      if (strcmp(checkPtr->value().c_str(), value) != 0) {
        testValueCnt++;
      } else {
        break;
      }
      ASSERT(testValueCnt < MAX, "Incorrect value found.");
    }
    dunit::sleep(SLEEP);
  }
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char* name, const char* key, const char* val,
                  int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyEntry() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

void createPooledRegion(const char* name, bool ackMode, const char* locators,
                        const char* poolname,
                        bool clientNotificationEnabled = false,
                        bool cachingEnable = true) {
  LOG("createRegion_Pool() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  auto regPtr =
      getHelper()->createPooledRegion(name, ackMode, locators, poolname,
                                      cachingEnable, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Pooled Region created.");
}
void createEntry(const char* name, const char* key, const char* value) {
  LOG("createEntry() entered.");
  fprintf(stdout, "Creating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Create entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(!regPtr->containsKey(keyPtr),
         "Key should not have been found in region.");
  ASSERT(!regPtr->containsValueForKey(keyPtr),
         "Value should not have been found in region.");

  // regPtr->create( keyPtr, valPtr );
  regPtr->put(keyPtr, valPtr);
  LOG("Created entry.");

  verifyEntry(name, key, value);
  LOG("Entry created.");
}

void updateEntry(const char* name, const char* key, const char* value) {
  LOG("updateEntry() entered.");
  fprintf(stdout, "Updating entry -- key: %s  value: %s in region %s\n", key,
          value, name);
  fflush(stdout);
  // Update entry, verify entry is correct
  auto keyPtr = CacheableKey::create(key);
  auto valPtr = CacheableString::create(value);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  ASSERT(regPtr->containsKey(keyPtr), "Key should have been found in region.");

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  verifyEntry(name, key, value);
  LOG("Entry updated.");
}

void doNetsearch(const char* name, const char* key, const char* value) {
  LOG("doNetsearch() entered.");
  fprintf(
      stdout,
      "Netsearching for entry -- key: %s  expecting value: %s in region %s\n",
      key, value, name);
  fflush(stdout);
  // Get entry created in Process A, verify entry is correct
  auto keyPtr = CacheableKey::create(key);

  auto regPtr = getHelper()->getRegion(name);
  fprintf(stdout, "netsearch  region %s\n", regPtr->getName().c_str());
  fflush(stdout);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto checkPtr = std::dynamic_pointer_cast<CacheableString>(
      regPtr->get(keyPtr));  // force a netsearch

  if (checkPtr != nullptr) {
    LOG("checkPtr is not null");
    char buf[1024];
    sprintf(buf, "In net search, get returned %s for key %s",
            checkPtr->value().c_str(), key);
    LOG(buf);
  } else {
    LOG("checkPtr is nullptr");
  }
  verifyEntry(name, key, value);
  LOG("Netsearch complete.");
}

std::vector<char*> storeEndPoints(const char* points) {
  std::vector<char*> endpointNames;
  if (points != nullptr) {
    char* ep = strdup(points);
    char* token = strtok(ep, ",");
    while (token) {
      endpointNames.push_back(token);
      token = strtok(nullptr, ",");
    }
  }
  ASSERT(endpointNames.size() == 3, "There should be 3 end points");
  return endpointNames;
}

std::vector<char*> endpointNames = storeEndPoints(endPoints);
const char* keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char* vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char* nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char* regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
const bool NO_ACK = false;

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) CacheHelper::initServer(1);
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, SetupClient1PooledLocator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__");
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__");
    LOG("SetupClient1PooledLocator complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, SetupClient2PooledLocator)
  {
    initClient(true);
    createPooledRegion(regionNames[0], USE_ACK, locatorsG, "__TEST_POOL1__");
    createPooledRegion(regionNames[1], NO_ACK, locatorsG, "__TEST_POOL1__");
    LOG("StepTwo complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, Client1CreateEntries)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[1], keys[2], vals[2]);
    LOG("Client1CreateEntries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, Client2CreateEntries)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    doNetsearch(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("Client2CreateEntries complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer2and3)
  {
    if (isLocalServer) CacheHelper::initServer(2);
    LOG("SERVER2 started");
    if (isLocalServer) CacheHelper::initServer(3);
    LOG("SERVER3 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, Client1UpdateEntries)
  {
    doNetsearch(regionNames[0], keys[1], vals[1]);
    doNetsearch(regionNames[1], keys[3], vals[3]);
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    LOG("Client1UpdateEntries complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT2, Client2UpdateEntries)
  {
    doNetsearch(regionNames[0], keys[0], vals[0]);
    doNetsearch(regionNames[1], keys[2], vals[2]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[3], nvals[3]);
    LOG("Client2UpdateEntries complete.");
  }
END_TASK_DEFINITION
DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2and3)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
    if (isLocalServer) {
      CacheHelper::closeServer(3);
      LOG("SERVER3 stopped");
    }
  }
END_TASK_DEFINITION

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_THINCLIENTFAILOVER3_H_
