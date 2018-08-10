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

#pragma once

#include "geode_defs.hpp"
#include "begin_native.hpp"
#include "end_native.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      /// <summary>
      /// Static class containing the classIds of the built-in cacheable types.
      /// </summary>
      [Obsolete("Used only to expose types to old integration tests.")]
      private ref class GeodeClassIds
      {
      public:

        /// <summary>
        /// ClassId of <c>Properties</c> class
        /// </summary>
        literal System::UInt32 Properties =
          static_cast<System::UInt32>(apache::geode::client::DSCode::Properties) + 0x80000000;

        /// <summary>        
        /// ClassId of <c>CharArray</c> class
        /// </summary>
        literal System::UInt32 CharArray =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CharArray) + 0x80000000;

        /// <summary>
        /// ClassId of <c>BooleanArray</c> class
        /// </summary>
        literal System::UInt32 BooleanArray =
          static_cast<System::UInt32>(apache::geode::client::DSCode::BooleanArray) + 0x80000000;

        /// <summary>
        /// ClassId of <c>RegionAttributes</c> class
        /// </summary>
        literal System::UInt32 RegionAttributes =
          static_cast<System::UInt32>(apache::geode::client::DSCode::RegionAttributes) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableUndefined</c> class
        /// Implementation note: this has DSFID of FixedIDByte hence a
        /// different increment.
        /// </summary>
        literal System::UInt32 CacheableUndefined =
          static_cast<System::UInt32>(apache::geode::client::DSFid::CacheableUndefined) + 0xa0000000;

        literal System::Int32 EnumInfo =
          static_cast<System::UInt32>(apache::geode::client::DSFid::EnumInfo);

        /// <summary>
        /// ClassId of <c>Struct</c> class
        /// </summary>
        literal System::UInt32 Struct =
          static_cast<System::UInt32>(apache::geode::client::DSFid::Struct) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class
        /// </summary>
        literal System::UInt32 CacheableString =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableString) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class for huge strings
        /// </summary>
        literal System::UInt32 CacheableStringHuge =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableStringHuge) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableBytes</c> class
        /// </summary>
        literal System::UInt32 CacheableBytes =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableBytes) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt16Array</c> class
        /// </summary>
        literal System::UInt32 CacheableInt16Array =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableInt16Array) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt32Array</c> class
        /// </summary>
        literal System::UInt32 CacheableInt32Array =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableInt32Array) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt64Array</c> class
        /// </summary>
        literal System::UInt32 CacheableInt64Array =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableInt64Array) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableFloatArray</c> class
        /// </summary>
        literal System::UInt32 CacheableFloatArray =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableFloatArray) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableDoubleArray</c> class
        /// </summary>
        literal System::UInt32 CacheableDoubleArray =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableDoubleArray) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableVector</c> class for object arrays
        /// </summary>
        literal System::UInt32 CacheableObjectArray =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableObjectArray) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableBoolean</c> class
        /// </summary>
        literal System::UInt32 CacheableBoolean =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableBoolean) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt16</c> class for wide-characters
        /// </summary>
        literal System::UInt32 CacheableCharacter =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableCharacter) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableByte</c> class
        /// </summary>
        literal System::UInt32 CacheableByte =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableByte) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt16</c> class
        /// </summary>
        literal System::UInt32 CacheableInt16 =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableInt16) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt32</c> class
        /// </summary>
        literal System::UInt32 CacheableInt32 =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableInt32) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt64</c> class
        /// </summary>
        literal System::UInt32 CacheableInt64 =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableInt64) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableFloat</c> class
        /// </summary>
        literal System::UInt32 CacheableFloat =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableFloat) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableDouble</c> class
        /// </summary>
        literal System::UInt32 CacheableDouble =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableDouble) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableDate</c> class
        /// </summary>
        literal System::UInt32 CacheableDate =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableDate) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableFileName</c> class
        /// </summary>
        literal System::UInt32 CacheableFileName =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableFileName) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableStringArray</c> class
        /// </summary>
        literal System::UInt32 CacheableStringArray =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableStringArray) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableVector</c> class
        /// </summary>
        literal System::UInt32 CacheableVector =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableVector) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableStack</c> class
        /// </summary>
        literal System::UInt32 CacheableStack =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableStack) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableArrayList</c> class
        /// </summary>
        literal System::UInt32 CacheableArrayList =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableArrayList) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableArrayList</c> class
        /// </summary>
        literal System::UInt32 CacheableLinkedList =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableLinkedList) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableHashSet</c> class
        /// </summary>
        literal System::UInt32 CacheableHashSet =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableHashSet) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableLinkedHashSet</c> class
        /// </summary>
        literal System::UInt32 CacheableLinkedHashSet =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableLinkedHashSet) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableHashMap</c> class
        /// </summary>
        literal System::UInt32 CacheableHashMap =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableHashMap) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableHashTable</c> class
        /// </summary>
        literal System::UInt32 CacheableHashTable =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableHashTable) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableIdentityHashMap</c> class
        /// </summary>
        literal System::UInt32 CacheableIdentityHashMap =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableIdentityHashMap) + 0x80000000;

        /// <summary>
        /// Not used.
        /// </summary>
        literal System::UInt32 CacheableTimeUnit =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableTimeUnit) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class for null strings
        /// </summary>
        literal System::UInt32 CacheableNullString =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableNullString) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class for ASCII strings
        /// </summary>
        literal System::UInt32 CacheableASCIIString =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableASCIIString) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class for huge ASCII strings
        /// </summary>
        literal System::UInt32 CacheableASCIIStringHuge =
          static_cast<System::UInt32>(apache::geode::client::DSCode::CacheableASCIIStringHuge) + 0x80000000;


        // Built-in managed types.

        /// <summary>
        /// ClassId of <c>CacheableObject</c> class
        /// </summary>
        literal System::UInt32 CacheableManagedObject = 
          static_cast<System::UInt32>(apache::geode::client::InternalId::CacheableManagedObject) + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableObjectXml</c> class
        /// </summary>
        literal System::UInt32 CacheableManagedObjectXml = 
          static_cast<System::UInt32>(apache::geode::client::InternalId::CacheableManagedObjectXml) + 0x80000000;

      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


