// RUN: %dxc %s /T ps_6_0 -opt-disable sink -HV 2018 | FileCheck %s

// Make sure the selects are NOT sunk into a single block

cbuffer cb {
    bool cond[6];
    bool br[6];
    float a[6];
}

float main() : SV_Target {
    float val = 0;
    float ret = 0;
    // CHECK: br
    if (br[0]) {
        // CHECK: select
        val = cond[0] ? 0 : a[0];
        // CHECK: br
        if (br[1]) {
            // CHECK: select
            val = cond[1] ? val : a[1];
            // CHECK: br
            if (br[2]) {
                // CHECK: select
                val = cond[2] ? val : a[2];
                // CHECK: br
                if (br[3]) {
                    // CHECK: select
                    val = cond[3] ? val : a[3];
                    ret = val;
                }
            }
        }
    }
    // CHECK: phi

    return ret;
}
