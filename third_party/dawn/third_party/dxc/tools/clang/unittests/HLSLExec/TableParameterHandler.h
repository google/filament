#ifndef TABLE_PARAMETER_HANDLER_H
#define TABLE_PARAMETER_HANDLER_H

#include <Verify.h>
#include <WexString.h>
#include <WexTestClass.h>
#include <memory>
#include <string>
#include <vector>
#include <wchar.h>
#include <windows.h> // For LPCWSTR

#include "dxc/Support/Global.h" // For DXASSERT_ARGS
#include "dxc/Test/HlslTestUtils.h"

// Parameter representation for taef data-driven tests
struct TableParameter {
  LPCWSTR m_name;
  enum TableParameterType {
    INT8,
    INT16,
    INT32,
    UINT,
    FLOAT,
    HALF,
    DOUBLE,
    STRING,
    BOOL,
    INT8_TABLE,
    INT16_TABLE,
    INT32_TABLE,
    FLOAT_TABLE,
    HALF_TABLE,
    DOUBLE_TABLE,
    STRING_TABLE,
    UINT8_TABLE,
    UINT16_TABLE,
    UINT32_TABLE,
    BOOL_TABLE
  };
  TableParameter(LPCWSTR name, TableParameterType type, bool required)
      : m_name(name), m_type(type), m_required(required) {}
  TableParameterType m_type;
  bool m_required; // required parameter
  int8_t m_int8;
  int16_t m_int16;
  int m_int32;
  unsigned int m_uint;
  float m_float;
  uint16_t m_half; // no such thing as half type in c++. Use int16 instead
  double m_double;
  bool m_bool;
  WEX::Common::String m_str;
  std::vector<int8_t> m_int8Table;
  std::vector<int16_t> m_int16Table;
  std::vector<int> m_int32Table;
  std::vector<uint8_t> m_uint8Table;
  std::vector<uint16_t> m_uint16Table;
  std::vector<unsigned int> m_uint32Table;
  std::vector<float> m_floatTable;
  std::vector<uint16_t> m_halfTable; // no such thing as half type in c++
  std::vector<double> m_doubleTable;
  std::vector<bool> m_boolTable;
  std::vector<WEX::Common::String> m_StringTable;
};

class TableParameterHandler {
private:
  HRESULT ParseTableRow();

public:
  TableParameter *m_table;
  size_t m_tableSize;
  TableParameterHandler(TableParameter *pTable, size_t size);

  TableParameter *GetTableParamByName(LPCWSTR name);
  void clearTableParameter();

  template <class T1> std::vector<T1> *GetDataArray(LPCWSTR name);
};

// Static helpers
static bool IsHexString(PCWSTR str, uint16_t *value) {
  std::wstring wString(str);
  wString.erase(std::remove(wString.begin(), wString.end(), L' '),
                wString.end());
  LPCWSTR wstr = wString.c_str();
  if (wcsncmp(wstr, L"0x", 2) == 0 || wcsncmp(wstr, L"0b", 2) == 0) {
    *value = (uint16_t)wcstol(wstr, NULL, 0);
    return true;
  }
  return false;
}

static HRESULT ParseDataToFloat(PCWSTR str, float &value) {
  std::wstring wString(str);
  wString.erase(std::remove(wString.begin(), wString.end(), L' '),
                wString.end());
  wString.erase(std::remove(wString.begin(), wString.end(), L'\n'),
                wString.end());
  PCWSTR wstr = wString.data();
  if (_wcsicmp(wstr, L"NaN") == 0) {
    value = NAN;
  } else if (_wcsicmp(wstr, L"-inf") == 0) {
    value = -(INFINITY);
  } else if (_wcsicmp(wstr, L"inf") == 0) {
    value = INFINITY;
  } else if (_wcsicmp(wstr, L"-denorm") == 0) {
    value = -(FLT_MIN / 2);
  } else if (_wcsicmp(wstr, L"denorm") == 0) {
    value = FLT_MIN / 2;
  } else if (_wcsicmp(wstr, L"-0.0f") == 0 || _wcsicmp(wstr, L"-0.0") == 0 ||
             _wcsicmp(wstr, L"-0") == 0) {
    value = -0.0f;
  } else if (_wcsicmp(wstr, L"0.0f") == 0 || _wcsicmp(wstr, L"0.0") == 0 ||
             _wcsicmp(wstr, L"0") == 0) {
    value = 0.0f;
  } else if (_wcsnicmp(wstr, L"0x", 2) ==
             0) { // For hex values, take values literally
    unsigned temp_i = std::stoul(wstr, nullptr, 16);
    value = (float &)temp_i;
  } else {
    // evaluate the expression of wstring
    double val = _wtof(wstr);
    if (val == 0) {
      hlsl_test::LogErrorFmt(L"Failed to parse parameter %s to float", wstr);
      return E_FAIL;
    }
    value = (float)val;
  }
  return S_OK;
}

static HRESULT ParseDataToUint(PCWSTR str, unsigned int &value) {
  std::wstring wString(str);
  wString.erase(std::remove(wString.begin(), wString.end(), L' '),
                wString.end());
  PCWSTR wstr = wString.data();
  // evaluate the expression of string
  if (_wcsicmp(wstr, L"0") == 0 || _wcsicmp(wstr, L"0x00000000") == 0) {
    value = 0;
    return S_OK;
  }
  wchar_t *end;
  unsigned int val = std::wcstoul(wstr, &end, 0);
  if (val == 0) {
    hlsl_test::LogErrorFmt(L"Failed to parse parameter %s to int", wstr);
    return E_FAIL;
  }
  value = val;
  return S_OK;
}

static HRESULT ParseDataToVectorFloat(PCWSTR str, float *ptr, size_t count) {
  std::wstring wstr(str);
  size_t curPosition = 0;
  // parse a string of dot product separated by commas
  for (size_t i = 0; i < count; ++i) {
    size_t nextPosition = wstr.find(L",", curPosition);
    if (FAILED(ParseDataToFloat(
            wstr.substr(curPosition, nextPosition - curPosition).data(),
            *(ptr + i)))) {
      return E_FAIL;
    }
    curPosition = nextPosition + 1;
  }
  return S_OK;
}

static HRESULT ParseDataToVectorHalf(PCWSTR str, uint16_t *ptr, size_t count) {
  std::wstring wstr(str);
  size_t curPosition = 0;
  // parse a string of dot product separated by commas
  for (size_t i = 0; i < count; ++i) {
    size_t nextPosition = wstr.find(L",", curPosition);
    float floatValue;
    if (FAILED(ParseDataToFloat(
            wstr.substr(curPosition, nextPosition - curPosition).data(),
            floatValue))) {
      return E_FAIL;
    }
    *(ptr + i) = ConvertFloat32ToFloat16(floatValue);
    curPosition = nextPosition + 1;
  }
  return S_OK;
}

static HRESULT ParseDataToVectorUint(PCWSTR str, unsigned int *ptr,
                                     size_t count) {
  std::wstring wstr(str);
  size_t curPosition = 0;
  // parse a string of dot product separated by commas
  for (size_t i = 0; i < count; ++i) {
    size_t nextPosition = wstr.find(L",", curPosition);
    if (FAILED(ParseDataToUint(
            wstr.substr(curPosition, nextPosition - curPosition).data(),
            *(ptr + i)))) {
      return E_FAIL;
    }
    curPosition = nextPosition + 1;
  }
  return S_OK;
}

#endif // TABLE_PARAMETER_HANDLER_H
