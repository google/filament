#!/usr/bin/env python3

import os
import sys
current_dir = os.path.abspath(os.path.dirname(__file__))
out_dir = 'out/cmake-release/samples'
sys.path.append(os.path.abspath(out_dir))
import pyfilament as filament


# res = filament.add(1, 2)

# print(res)

config = filament.Config()
config.title = "Material Sandbox"
config.ibl_dir = os.path.join(current_dir, '../envs/desert/')
model = os.path.join(os.environ['HOME'],
                     'p/surreal/datageneration/smpl_data/basicModel_f_lbs_10_207_0_v1.0.2.fbx')
print(config.title, config.backend)
filament.main(config, model)
