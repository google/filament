# Check xunit output
# RUN: %{lit} --xunit-xml-output %t.xunit.xml --xml-include-test-output %{inputs}/test-data
# RUN: FileCheck < %t.xunit.xml %s

# CHECK: <?xml version="1.0" encoding="UTF-8" ?>
# CHECK: <testsuites>
# CHECK: <testsuite name='test-data' tests='2' failures='0'>
# CHECK: <testcase classname='test-data.test-data' name='metrics.ini' time='0.{{[0-9]+}}'>
# CHECK:   <system-out>
# CHECK:        Test passed.
# CHECK:   </system-out>
# CHECK: <testcase classname='test-data.test-data' name='utf8_output_message.ini' time='0.{{[0-9]+}}'>
# CHECK:   <system-out>
# CHECK:        This test is 🔥
# CHECK:   </system-out>
# CHECK: </testcase>
# CHECK: </testsuite>
# CHECK: </testsuites>
