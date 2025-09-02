<?php

// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

/**
 * MapField and MapFieldIter are used by generated protocol message classes to
 * manipulate map fields.
 */

namespace Google\Protobuf\Internal;

use Traversable;

/**
 * MapField is used by generated protocol message classes to manipulate map
 * fields. It can be used like native PHP array.
 */
class MapField implements \ArrayAccess, \IteratorAggregate, \Countable
{
    /**
     * @ignore
     */
    private $container;
    /**
     * @ignore
     */
    private $key_type;
    /**
     * @ignore
     */
    private $value_type;
    /**
     * @ignore
     */
    private $klass;
    /**
     * @ignore
     */
    private $legacy_klass;

    /**
     * Constructs an instance of MapField.
     *
     * @param long $key_type Type of the stored key element.
     * @param long $value_type Type of the stored value element.
     * @param string $klass Message/Enum class name of value instance
     * (message/enum fields only).
     * @ignore
     */
    public function __construct($key_type, $value_type, $klass = null)
    {
        $this->container = [];
        $this->key_type = $key_type;
        $this->value_type = $value_type;
        $this->klass = $klass;

        if ($this->value_type == GPBType::MESSAGE) {
            $pool = DescriptorPool::getGeneratedPool();
            $desc = $pool->getDescriptorByClassName($klass);
            if ($desc == NULL) {
                new $klass;  // No msg class instance has been created before.
                $desc = $pool->getDescriptorByClassName($klass);
            }
            $this->klass = $desc->getClass();
            $this->legacy_klass = $desc->getLegacyClass();
        }
    }

    /**
     * @ignore
     */
    public function getKeyType()
    {
        return $this->key_type;
    }

    /**
     * @ignore
     */
    public function getValueType()
    {
        return $this->value_type;
    }

    /**
     * @ignore
     */
    public function getValueClass()
    {
        return $this->klass;
    }

    /**
     * @ignore
     */
    public function getLegacyValueClass()
    {
        return $this->legacy_klass;
    }

    /**
     * Return the element at the given key.
     *
     * This will also be called for: $ele = $arr[$key]
     *
     * @param int|string $key The key of the element to be fetched.
     * @return object The stored element at given key.
     * @throws \ErrorException Invalid type for index.
     * @throws \ErrorException Non-existing index.
     * @todo need to add return type mixed (require update php version to 8.0)
     */
    #[\ReturnTypeWillChange]
    public function offsetGet($key)
    {
        return $this->container[$key];
    }

    /**
     * Assign the element at the given key.
     *
     * This will also be called for: $arr[$key] = $value
     *
     * @param int|string $key The key of the element to be fetched.
     * @param object $value The element to be assigned.
     * @return void
     * @throws \ErrorException Invalid type for key.
     * @throws \ErrorException Invalid type for value.
     * @throws \ErrorException Non-existing key.
     * @todo need to add return type void (require update php version to 7.1)
     */
    #[\ReturnTypeWillChange]
    public function offsetSet($key, $value)
    {
        $this->checkKey($this->key_type, $key);

        switch ($this->value_type) {
            case GPBType::SFIXED32:
            case GPBType::SINT32:
            case GPBType::INT32:
            case GPBType::ENUM:
                GPBUtil::checkInt32($value);
                break;
            case GPBType::FIXED32:
            case GPBType::UINT32:
                GPBUtil::checkUint32($value);
                break;
            case GPBType::SFIXED64:
            case GPBType::SINT64:
            case GPBType::INT64:
                GPBUtil::checkInt64($value);
                break;
            case GPBType::FIXED64:
            case GPBType::UINT64:
                GPBUtil::checkUint64($value);
                break;
            case GPBType::FLOAT:
                GPBUtil::checkFloat($value);
                break;
            case GPBType::DOUBLE:
                GPBUtil::checkDouble($value);
                break;
            case GPBType::BOOL:
                GPBUtil::checkBool($value);
                break;
            case GPBType::STRING:
                GPBUtil::checkString($value, true);
                break;
            case GPBType::MESSAGE:
                if (is_null($value)) {
                  trigger_error("Map element cannot be null.", E_USER_ERROR);
                }
                GPBUtil::checkMessage($value, $this->klass);
                break;
            default:
                break;
        }

        $this->container[$key] = $value;
    }

    /**
     * Remove the element at the given key.
     *
     * This will also be called for: unset($arr)
     *
     * @param int|string $key The key of the element to be removed.
     * @return void
     * @throws \ErrorException Invalid type for key.
     * @todo need to add return type void (require update php version to 7.1)
     */
    #[\ReturnTypeWillChange]
    public function offsetUnset($key)
    {
        $this->checkKey($this->key_type, $key);
        unset($this->container[$key]);
    }

    /**
     * Check the existence of the element at the given key.
     *
     * This will also be called for: isset($arr)
     *
     * @param int|string $key The key of the element to be removed.
     * @return bool True if the element at the given key exists.
     * @throws \ErrorException Invalid type for key.
     */
    public function offsetExists($key): bool
    {
        $this->checkKey($this->key_type, $key);
        return isset($this->container[$key]);
    }

    /**
     * @ignore
     */
    public function getIterator(): Traversable
    {
        return new MapFieldIter($this->container, $this->key_type);
    }

    /**
     * Return the number of stored elements.
     *
     * This will also be called for: count($arr)
     *
     * @return integer The number of stored elements.
     */
    public function count(): int
    {
        return count($this->container);
    }

    /**
     * @ignore
     */
    private function checkKey($key_type, &$key)
    {
        switch ($key_type) {
            case GPBType::SFIXED32:
            case GPBType::SINT32:
            case GPBType::INT32:
                GPBUtil::checkInt32($key);
                break;
            case GPBType::FIXED32:
            case GPBType::UINT32:
                GPBUtil::checkUint32($key);
                break;
            case GPBType::SFIXED64:
            case GPBType::SINT64:
            case GPBType::INT64:
                GPBUtil::checkInt64($key);
                break;
            case GPBType::FIXED64:
            case GPBType::UINT64:
                GPBUtil::checkUint64($key);
                break;
            case GPBType::BOOL:
                GPBUtil::checkBool($key);
                break;
            case GPBType::STRING:
                GPBUtil::checkString($key, true);
                break;
            default:
                trigger_error(
                    "Given type cannot be map key.",
                    E_USER_ERROR);
                break;
        }
    }
}
