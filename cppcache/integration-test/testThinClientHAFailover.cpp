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

#include <string>

#define ROOT_NAME "testThinClientHAFailover"
#define ROOT_SCOPE DISTRIBUTED_ACK

#include "CacheHelper.hpp"

using apache::geode::client::CacheableKey;
using apache::geode::client::CacheableString;
using apache::geode::client::CacheHelper;
using apache::geode::client::Properties;

bool isLocalServer = false;

CacheHelper *cacheHelper = nullptr;
int g_redundancyLevel = 0;

#define CLIENT1 s1p1
#define CLIENT2 s1p2
#define SERVER1 s2p1
#define SERVER2 s2p2

static bool isLocator = false;
static int numberOfLocators = 1;
const char *locatorsG =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);

void initClient(int);
void initClient(int redundancyLevel) {
  static bool s_isGridClient = true;

  s_isGridClient = !s_isGridClient;
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (s_isGridClient) {
      config->insert("grid-client", "true");
    }
    cacheHelper = new CacheHelper(redundancyLevel, config);
  }
  g_redundancyLevel = redundancyLevel;
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void initClient();
void initClient()  // For POOL API redendency will be set at POOL level.
{
  static bool s_isGridClient = true;
  s_isGridClient = !s_isGridClient;
  if (cacheHelper == nullptr) {
    auto config = Properties::create();
    if (s_isGridClient) {
      config->insert("grid-client", "true");
    }
    cacheHelper = new CacheHelper(true, config);
  }
  ASSERT(cacheHelper, "Failed to create a CacheHelper client instance.");
}

void cleanProc();
void cleanProc() {
  if (cacheHelper != nullptr) {
    delete cacheHelper;
    cacheHelper = nullptr;
  }
}

CacheHelper *getHelper();
CacheHelper *getHelper() {
  ASSERT(cacheHelper != nullptr, "No cacheHelper initialized.");
  return cacheHelper;
}

void _verifyEntry(const char *, const char *, const char *, bool, bool);
void _verifyEntry(const char *name, const char *key, const char *val,
                  bool noKey, bool isCreated = false) {
  // Verify key and value exist in this region, in this process.
  const char *value = val ? val : "";
  char *buf =
      reinterpret_cast<char *>(malloc(1024 + strlen(key) + strlen(value)));
  ASSERT(buf, "Unable to malloc buffer for logging.");
  if (!isCreated) {
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
  }
  free(buf);

  auto regPtr = getHelper()->getRegion(name);
  ASSERT(regPtr != nullptr, "Region not found.");

  auto keyPtr = CacheableKey::create(key);

  // if the region is no ack, then we may need to wait...
  if (!isCreated) {
    if (noKey == false) {  // need to find the key!
      ASSERT(regPtr->containsKey(keyPtr), "Key not found in region.");
    }
    if (val != nullptr) {  // need to have a value!
      // ASSERT( regPtr->containsValueForKey( keyPtr ), "Value not found in
      // region." );
    }
  }

  // loop up to MAX times, testing condition
  uint32_t MAX = 100;
  //  changed sleep from 10 ms
  uint32_t SLEEP = 10;  // milliseconds
  uint32_t containsKeyCnt = 0;
  uint32_t containsValueCnt = 0;
  uint32_t testValueCnt = 0;

  for (int i = MAX; i >= 0; i--) {
    if (isCreated) {
      if (!regPtr->containsKey(keyPtr)) {
        containsKeyCnt++;
      } else {
        break;
      }
      ASSERT(containsKeyCnt < MAX, "Key has not been created in region.");
    } else {
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
        LOG("In verify loop, get returned " + checkPtr->value() + " for key " +
            key);
        if (strcmp(checkPtr->value().c_str(), value) != 0) {
          testValueCnt++;
        } else {
          break;
        }
        ASSERT(testValueCnt < MAX, "Incorrect value found.");
      }
    }
    dunit::sleep(SLEEP);
  }
}

#define verifyEntry(x, y, z) _verifyEntry(x, y, z, __LINE__)

void _verifyEntry(const char *, const char *, const char *, int);
void _verifyEntry(const char *name, const char *key, const char *val,
                  int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyEntry() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, val, false);
  LOG("Entry verified.");
}

#define verifyCreated(x, y) _verifyCreated(x, y, __LINE__)

void _verifyCreated(const char *, const char *, int);
void _verifyCreated(const char *name, const char *key, int line) {
  char logmsg[1024];
  sprintf(logmsg, "verifyCreated() called from %d.\n", line);
  LOG(logmsg);
  _verifyEntry(name, key, nullptr, false, true);
  LOG("Entry created.");
}

void createRegion(const char *, bool, bool);
void createRegion(const char *name, bool ackMode,
                  bool clientNotificationEnabled = false) {
  LOG("createRegion() entered.");
  fprintf(stdout, "Creating region --  %s  ackMode is %d\n", name, ackMode);
  fflush(stdout);
  char *endpoints = nullptr;
  // ack, caching
  auto regPtr = getHelper()->createRegion(name, ackMode, true, nullptr,
                                          endpoints, clientNotificationEnabled);
  ASSERT(regPtr != nullptr, "Failed to create region.");
  LOG("Region created.");
}
void createEntry(const char *, const char *, const char *);
void createEntry(const char *name, const char *key, const char *value) {
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

void updateEntry(const char *, const char *, const char *);
void updateEntry(const char *name, const char *key, const char *value) {
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
  // ASSERT( regPtr->containsValueForKey( keyPtr ), "Value should have been
  // found in region." );

  regPtr->put(keyPtr, valPtr);
  LOG("Put entry.");

  verifyEntry(name, key, value);
  LOG("Entry updated.");
}

void doNetsearch(const char *, const char *, const char *);
void doNetsearch(const char *name, const char *key, const char *value) {
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

  // ASSERT( !regPtr->containsKey( keyPtr ), "Key should not have been found in
  // region." );
  // ASSERT( !regPtr->containsValueForKey( keyPtr ), "Value should not have been
  // found in region." );

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

const char *keys[] = {"Key-1", "Key-2", "Key-3", "Key-4"};
const char *vals[] = {"Value-1", "Value-2", "Value-3", "Value-4"};
const char *nvals[] = {"New Value-1", "New Value-2", "New Value-3",
                       "New Value-4"};

const char *regionNames[] = {"DistRegionAck", "DistRegionNoAck"};

const bool USE_ACK = true;
void initClientAndRegion(int);
void initClientAndRegion(int redundancy) {
  auto pp = Properties::create();
  g_redundancyLevel = redundancy;
  getHelper()->createPoolWithLocators("__TESTPOOL1_", locatorsG, true,
                                      redundancy);
  getHelper()->createPoolWithLocators("__TESTPOOL2_", locatorsG, true,
                                      redundancy);
  getHelper()->createRegionAndAttachPool(regionNames[0], USE_ACK,
                                         "__TESTPOOL1_", true);
  getHelper()->createRegionAndAttachPool(regionNames[1], USE_ACK,
                                         "__TESTPOOL2_", true);
}

#include "LocatorHelper.hpp"

DUNIT_TASK_DEFINITION(SERVER1, CreateServer1)
  {
    if (isLocalServer) {
      CacheHelper::initServer(1, "cacheserver_notify_subscription.xml");
    }
    LOG("SERVER1 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1_R1)
  {
    initClient();
    LOG("Initialized client with redundancy level 1.");
    initClientAndRegion(1);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitClient2_R1)
  {
    initClient();
    initClientAndRegion(1);
    LOG("Initialized client with redundancy level 1.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, InitClient1_R3)
  {
    initClient();
    initClientAndRegion(3);
    LOG("Initialized client with redundancy level 3.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, InitClient2_R3)
  {
    initClient();
    initClientAndRegion(3);
    LOG("Initialized client with redundancy level 3.");
  }
END_TASK_DEFINITION

/*DUNIT_TASK_DEFINITION( CLIENT1, StepOne )
{
  initClientAndRegion(1);
  LOG( "StepOne complete." );
}
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION( CLIENT2, StepTwo )
{
  createRegion( regionNames[0], USE_ACK, false );
  createRegion( regionNames[1], NO_ACK, false );

  LOG( "StepTwo complete." );
}
END_TASK_DEFINITION
*/
DUNIT_TASK_DEFINITION(SERVER2, CreateServer2)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml");
    }
    LOG("SERVER2 started");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, RegisterKeys)
  {
    auto regPtr0 = getHelper()->getRegion(regionNames[0]);
    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    auto keyPtr0 = CacheableKey::create(keys[0]);
    auto keyPtr2 = CacheableKey::create(keys[2]);

    std::vector<std::shared_ptr<CacheableKey>> keys0, keys1;
    keys0.push_back(keyPtr0);
    keys1.push_back(keyPtr2);
    if (g_redundancyLevel > 1) {
      regPtr0->registerKeys(keys0);
      regPtr1->registerKeys(keys1);
    } else {
      regPtr0->registerKeys(keys0);
      regPtr1->registerKeys(keys1);
    }
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepThree)
  {
    createEntry(regionNames[0], keys[0], vals[0]);
    createEntry(regionNames[0], keys[1], vals[1]);
    createEntry(regionNames[1], keys[2], vals[2]);
    createEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepThree complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepFour)
  {
    verifyCreated(regionNames[0], keys[0]);
    verifyCreated(regionNames[1], keys[2]);
    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[1], keys[2], vals[2]);
    doNetsearch(regionNames[0], keys[1], vals[1]);
    doNetsearch(regionNames[1], keys[3], vals[3]);
    LOG("StepFour complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer1)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepFive)
  {
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    updateEntry(regionNames[1], keys[3], nvals[3]);
    SLEEP(1000);
    LOG("StepFive complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepSix)
  {
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], nvals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepSix complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer11)
  {
    CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                            locatorsG);
    LOG("SERVER1 started");
    SLEEP(5000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer2)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
    SLEEP(5000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CreateServer3)
  {
    CacheHelper::initServer(3, "cacheserver_notify_subscription3.xml",
                            locatorsG);
    LOG("SERVER3 started");
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepSeven)
  {
    updateEntry(regionNames[0], keys[0], vals[0]);
    updateEntry(regionNames[0], keys[1], vals[1]);
    updateEntry(regionNames[1], keys[2], vals[2]);
    updateEntry(regionNames[1], keys[3], vals[3]);
    SLEEP(1000);
    LOG("StepSeven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepEight)
  {
    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], vals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);

    auto regPtr1 = getHelper()->getRegion(regionNames[1]);

    auto keyPtr2 = CacheableKey::create(keys[2]);

    std::vector<std::shared_ptr<CacheableKey>> keys1;
    keys1.push_back(keyPtr2);
    regPtr1->unregisterKeys(keys1);
    LOG("StepEight complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CreateServer21)
  {
    if (isLocalServer) {
      CacheHelper::initServer(2, "cacheserver_notify_subscription2.xml",
                              locatorsG);
    }
    LOG("SERVER2 started");
    SLEEP(5000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer3)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(3);
      LOG("SERVER3 stopped");
    }
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepNine)
  {
    updateEntry(regionNames[0], keys[0], nvals[0]);
    updateEntry(regionNames[0], keys[1], nvals[1]);
    updateEntry(regionNames[1], keys[2], nvals[2]);
    updateEntry(regionNames[1], keys[3], nvals[3]);
    SLEEP(1000);
    LOG("StepNine complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTen)
  {
    verifyEntry(regionNames[0], keys[0], nvals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], vals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepTen complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER1, CloseServer11)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(1);
      LOG("SERVER1 stopped");
    }
    SLEEP(15000);
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, StepEleven)
  {
    updateEntry(regionNames[0], keys[0], vals[0]);
    updateEntry(regionNames[0], keys[1], vals[1]);
    updateEntry(regionNames[1], keys[2], vals[2]);
    updateEntry(regionNames[1], keys[3], vals[3]);
    SLEEP(1000);
    LOG("StepEleven complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, StepTwelve)
  {
    verifyEntry(regionNames[0], keys[0], vals[0]);
    verifyEntry(regionNames[0], keys[1], vals[1]);
    verifyEntry(regionNames[1], keys[2], vals[2]);
    verifyEntry(regionNames[1], keys[3], vals[3]);
    LOG("StepTwelve complete.");
  }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT1, CloseCache1)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(CLIENT2, CloseCache2)
  { cleanProc(); }
END_TASK_DEFINITION

DUNIT_TASK_DEFINITION(SERVER2, CloseServer21)
  {
    if (isLocalServer) {
      CacheHelper::closeServer(2);
      LOG("SERVER2 stopped");
    }
    SLEEP(5000);
  }
END_TASK_DEFINITION

void doThinClientHAFailover();
void doThinClientHAFailover() {
  for (int runNum = 0; runNum <= 3; runNum++) {
    CALL_TASK(CreateLocator1);
    CALL_TASK(CreateServer1_With_Locator_XML);

    int runType = runNum / 2;
    if (runType == 0) {
      CALL_TASK(InitClient1_R1);
      CALL_TASK(InitClient2_R1);
    } else {
      CALL_TASK(InitClient1_R3);
      CALL_TASK(InitClient2_R3);
    }
    // CALL_TASK( StepOne );
    // CALL_TASK( StepTwo );

    CALL_TASK(CreateServer2_With_Locator_XML);

    CALL_TASK(RegisterKeys);
    CALL_TASK(StepThree);
    CALL_TASK(StepFour);
    CALL_TASK(CloseServer1);
    CALL_TASK(StepFive);
    CALL_TASK(StepSix);
    CALL_TASK(CreateServer11);
    CALL_TASK(CloseServer2);
    CALL_TASK(CreateServer3);
    CALL_TASK(StepSeven);
    CALL_TASK(StepEight);
    CALL_TASK(CreateServer21);
    CALL_TASK(CloseServer3);
    CALL_TASK(StepNine);
    CALL_TASK(StepTen);
    CALL_TASK(CloseServer11);
    CALL_TASK(StepEleven);
    CALL_TASK(StepTwelve);
    CALL_TASK(CloseServer21);
    CALL_TASK(CloseCache1);
    CALL_TASK(CloseCache2);

    CALL_TASK(CloseLocator1);
    SLEEP(1000);
  }
}

DUNIT_MAIN
  { doThinClientHAFailover(); }
END_MAIN
