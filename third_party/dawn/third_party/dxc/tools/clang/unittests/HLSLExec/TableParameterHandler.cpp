#include "TableParameterHandler.h"
#include "dxc/Test/HlslTestUtils.h"

TableParameterHandler::TableParameterHandler(TableParameter *pTable,
                                             size_t size)
    : m_table(pTable), m_tableSize(size) {
  clearTableParameter();
  VERIFY_SUCCEEDED(ParseTableRow());
}

TableParameter *TableParameterHandler::GetTableParamByName(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &m_table[i];
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

void TableParameterHandler::clearTableParameter() {
  for (size_t i = 0; i < m_tableSize; ++i) {
    m_table[i].m_int32 = 0;
    m_table[i].m_uint = 0;
    m_table[i].m_double = 0;
    m_table[i].m_bool = false;
    m_table[i].m_str = WEX::Common::String();
  }
}

template <class T1>
std::vector<T1> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  return nullptr;
}

template <>
std::vector<int> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &(m_table[i].m_int32Table);
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

template <>
std::vector<int8_t> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &(m_table[i].m_int8Table);
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

template <>
std::vector<int16_t> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &(m_table[i].m_int16Table);
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

template <>
std::vector<unsigned int> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &(m_table[i].m_uint32Table);
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

template <>
std::vector<float> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &(m_table[i].m_floatTable);
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

template <>
std::vector<uint16_t> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &(m_table[i].m_halfTable);
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

template <>
std::vector<double> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &(m_table[i].m_doubleTable);
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

template <>
std::vector<bool> *TableParameterHandler::GetDataArray(LPCWSTR name) {
  for (size_t i = 0; i < m_tableSize; ++i) {
    if (_wcsicmp(name, m_table[i].m_name) == 0) {
      return &(m_table[i].m_boolTable);
    }
  }
  DXASSERT_ARGS(false, "Invalid Table Parameter Name %s", name);
  return nullptr;
}

HRESULT TableParameterHandler::ParseTableRow() {
  TableParameter *table = m_table;
  for (unsigned int i = 0; i < m_tableSize; ++i) {
    switch (table[i].m_type) {
    case TableParameter::INT8:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_int32)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int16
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_int8 = (int8_t)(table[i].m_int32);
      break;
    case TableParameter::INT16:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_int32)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int16
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_int16 = (short)(table[i].m_int32);
      break;
    case TableParameter::INT32:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_int32)) &&
          table[i].m_required) {
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::UINT:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_uint)) &&
          table[i].m_required) {
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::DOUBLE:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(
              table[i].m_name, table[i].m_double)) &&
          table[i].m_required) {
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::STRING:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_str)) &&
          table[i].m_required) {
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::BOOL:
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           table[i].m_str)) &&
          table[i].m_bool) {
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      break;
    case TableParameter::INT8_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {

        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      // TryGetValue does not suppport reading from int8
      table[i].m_int8Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_int8Table[j] = (int8_t)tempTable[j];
      }
      break;
    }
    case TableParameter::INT16_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      // TryGetValue does not suppport reading from int8
      table[i].m_int16Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_int16Table[j] = (int16_t)tempTable[j];
      }
      break;
    }
    case TableParameter::INT32_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_int32Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_int32Table[j] = tempTable[j];
      }
      break;
    }
    case TableParameter::UINT8_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {

        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      // TryGetValue does not suppport reading from int8
      table[i].m_int8Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_int8Table[j] = (uint8_t)tempTable[j];
      }
      break;
    }
    case TableParameter::UINT16_TABLE: {
      WEX::TestExecution::TestDataArray<int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      // TryGetValue does not suppport reading from int8
      table[i].m_uint16Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_uint16Table[j] = (uint16_t)tempTable[j];
      }
      break;
    }
    case TableParameter::UINT32_TABLE: {
      WEX::TestExecution::TestDataArray<unsigned int> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_uint32Table.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_uint32Table[j] = tempTable[j];
      }
      break;
    }
    case TableParameter::FLOAT_TABLE: {
      WEX::TestExecution::TestDataArray<WEX::Common::String> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_floatTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        ParseDataToFloat(tempTable[j], table[i].m_floatTable[j]);
      }
      break;
    }
    case TableParameter::HALF_TABLE: {
      WEX::TestExecution::TestDataArray<WEX::Common::String> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_halfTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        uint16_t value = 0;
        if (IsHexString(tempTable[j], &value)) {
          table[i].m_halfTable[j] = value;
        } else {
          float val;
          ParseDataToFloat(tempTable[j], val);
          if (isdenorm(val))
            table[i].m_halfTable[j] =
                signbit(val) ? Float16NegDenorm : Float16PosDenorm;
          else
            table[i].m_halfTable[j] = ConvertFloat32ToFloat16(val);
        }
      }
      break;
    }
    case TableParameter::DOUBLE_TABLE: {
      WEX::TestExecution::TestDataArray<double> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_doubleTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_doubleTable[j] = tempTable[j];
      }
      break;
    }
    case TableParameter::BOOL_TABLE: {
      WEX::TestExecution::TestDataArray<bool> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_boolTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_boolTable[j] = tempTable[j];
      }
      break;
    }
    case TableParameter::STRING_TABLE: {
      WEX::TestExecution::TestDataArray<WEX::Common::String> tempTable;
      if (FAILED(WEX::TestExecution::TestData::TryGetValue(table[i].m_name,
                                                           tempTable)) &&
          table[i].m_required) {
        // TryGetValue does not suppport reading from int8
        hlsl_test::LogErrorFmt(L"Failed to get %s", table[i].m_name);
        return E_FAIL;
      }
      table[i].m_StringTable.resize(tempTable.GetSize());
      for (size_t j = 0, end = tempTable.GetSize(); j != end; ++j) {
        table[i].m_StringTable[j] = tempTable[j];
      }
      break;
    }
    default:
      DXASSERT_NOMSG("Invalid Parameter Type");
    }
    if (errno == ERANGE) {
      hlsl_test::LogErrorFmt(L"got out of range value for table %s",
                             table[i].m_name);
      return E_FAIL;
    }
  }
  return S_OK;
}
