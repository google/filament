# CRAFTING PHYSICALLY-BASED MATERIALS

## BASE COLOR/sRGB
Defines the perceived color of an object (sometimes called **albedo**). More precisely:

   → the **diffuse color** of a **non-metallic** object\
   → the **specular color** of a **metallic object**

### BASE COLOR LUMINOSITY
<div style="display:flex;width:100%;flex-direction:column;font-size:12px;color:#aaa">
  <div style="display:flex;min-height:25px;min-width:10px;flex-grow:1;background-image:linear-gradient(to right, black, white);"></div>
  <div style="display:flex;width:100%;flex-grow:1;margin-top:10px">
    <div style="flex-grow:10">&nbsp </div>
    <div style="display:flex;flex-direction:row;flex-grow:230;border-style:none;text-align:center;padding:2px 0;align-items:center">
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
      <div style="display:flex;flex-direction:column">
        <div> Non-metal range </div>
        <div> 10 - 240 </div>
      </div>
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
    </div>
    <div style="flex-grow:15">&nbsp </div>
  </div>
  <div style="display:flex;width:100%;flex-grow:1;">
    <div style="flex-grow:170">&nbsp </div>
    <div style="display:flex;flex-direction:row;flex-grow:85;border-style:none;text-align:center;padding:2px 0;align-items:center">
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
      <div style="display:flex;flex-direction:column">
        <div> Metal range </div>
        <div> 170 - 255 </div>
      </div>
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
    </div>
  </div>
</div>

### METALLIC SAMPLES
<div style="overflow-x:auto;font-size:12px">
<table>
  <tbody>
    <tr>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#faf9f5">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#faf5f5">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#d6d1c8">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#c0bdba">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#cec8c2">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#fbd8b8">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#fedc9d">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#f4e4ad">
            </div>
        </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> Silver </td>
        <td style="border-color:transparent;padding:3px 10px"> Aluminum </td>
        <td style="border-color:transparent;padding:3px 10px"> Platinum </td>
        <td style="border-color:transparent;padding:3px 10px"> Iron </td>
        <td style="border-color:transparent;padding:3px 10px"> Titanium </td>
        <td style="border-color:transparent;padding:3px 10px"> Copper </td>
        <td style="border-color:transparent;padding:3px 10px"> Gold </td>
        <td style="border-color:transparent;padding:3px 10px"> Brass </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> 250,249,245 </td>
        <td style="border-color:transparent;padding:3px 10px"> 244,245,245 </td>
        <td style="border-color:transparent;padding:3px 10px"> 214,209,200 </td>
        <td style="border-color:transparent;padding:3px 10px"> 192,189,186 </td>
        <td style="border-color:transparent;padding:3px 10px"> 206,200,194 </td>
        <td style="border-color:transparent;padding:3px 10px"> 251,216,184 </td>
        <td style="border-color:transparent;padding:3px 10px"> 255,220,157 </td>
        <td style="border-color:transparent;padding:3px 10px"> 244,228,173 </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> #faf9f5 </td>
        <td style="border-color:transparent;padding:3px 10px"> #faf5f5 </td>
        <td style="border-color:transparent;padding:3px 10px"> #d6d1c8 </td>
        <td style="border-color:transparent;padding:3px 10px"> #c0bdba </td>
        <td style="border-color:transparent;padding:3px 10px"> #cec8c2 </td>
        <td style="border-color:transparent;padding:3px 10px"> #fbd8b8 </td>
        <td style="border-color:transparent;padding:3px 10px"> #fedc9d </td>
        <td style="border-color:transparent;padding:3px 10px"> #f4e4ad </td>
    </tr>
  </tbody>
</table>
</div>


### NON-METALLIC SAMPLES

<div style="overflow-x:auto;font-size:12px">
<table>
  <tbody>
    <tr>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#323232">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#353535">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#553d31">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#875c3c">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#7b824e">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#947d75">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#b1a884">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:#c0bfbb">
            </div>
        </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> Coal </td>
        <td style="border-color:transparent;padding:3px 10px"> Rubber </td>
        <td style="border-color:transparent;padding:3px 10px"> Mud </td>
        <td style="border-color:transparent;padding:3px 10px"> Wood </td>
        <td style="border-color:transparent;padding:3px 10px"> Vegetation </td>
        <td style="border-color:transparent;padding:3px 10px"> Brick </td>
        <td style="border-color:transparent;padding:3px 10px"> Sand </td>
        <td style="border-color:transparent;padding:3px 10px"> Concrete </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> 50,50,50 </td>
        <td style="border-color:transparent;padding:3px 10px"> 53,53,53 </td>
        <td style="border-color:transparent;padding:3px 10px"> 85,61,49 </td>
        <td style="border-color:transparent;padding:3px 10px"> 135,92,60 </td>
        <td style="border-color:transparent;padding:3px 10px"> 123,130,78 </td>
        <td style="border-color:transparent;padding:3px 10px"> 148,125,117 </td>
        <td style="border-color:transparent;padding:3px 10px"> 177,168,132 </td>
        <td style="border-color:transparent;padding:3px 10px"> 192,191,187 </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> #323232 </td>
        <td style="border-color:transparent;padding:3px 10px"> #353535 </td>
        <td style="border-color:transparent;padding:3px 10px"> #553d31 </td>
        <td style="border-color:transparent;padding:3px 10px"> #875c3c </td>
        <td style="border-color:transparent;padding:3px 10px"> #7b824e </td>
        <td style="border-color:transparent;padding:3px 10px"> #947d75 </td>
        <td style="border-color:transparent;padding:3px 10px"> #b1a884 </td>
        <td style="border-color:transparent;padding:3px 10px"> #c0bfbb </td>
    </tr>
  </tbody>
</table>
</div>

## METALLIC/GRAYSCALE
Defines whether a surface is **dielectric** (0.0, **non-metal**) or **conductor** (1.0, **metal**).
Pure, unweathered surfaces are rare and will be either **0.0** or **1.0**.
Rust is not a conductor.

<div style="overflow-x:auto;font-size:12px">
<table>
  <thead style="background-color:transparent">
    <tr style="border-color:transparent">
      <th>0.0</th><th>0.1</th><th>0.2</th><th>0.3</th><th>0.4</th><th>0.5</th><th>0.6</th><th>0.7</th><th>0.8</th><th>0.9</th><th>1.0</th>
    </tr>
  </thead>
  <tbody>
    <tr>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_00.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_01.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_02.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_03.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_04.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_05.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_06.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_07.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_08.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_09.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_grayscale_10.png"></img></td>
    </tr>
    <tr style="background-color:transparent">
      <td colspan="5" style="border-color:transparent">NON-METAL/DIELECTRIC</td>
      <td colspan="6" style="border-color:transparent;text-align:right">METAL/CONDUCTOR</td>
    </tr>
  </tbody>
</table>
</div>

## ROUGHNESS/GRAYSCALE
Defines the perceived **smoothness** (0.0) or **roughness** (1.0).
It is sometimes called **glossiness**.

### NON-METALLIC
<div style="overflow-x:auto;font-size:12px">
<table>
  <thead style="background-color:transparent">
    <tr style="border-color:transparent">
      <th>0.0</th><th>0.1</th><th>0.2</th><th>0.3</th><th>0.4</th><th>0.5</th><th>0.6</th><th>0.7</th><th>0.8</th><th>0.9</th><th>1.0</th>
    </tr>
  </thead>
  <tbody>
    <tr>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_00.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_01.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_02.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_03.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_04.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_05.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_06.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_07.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_08.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_09.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/non_metallic_10.png"></img></td>
    </tr>
  </tbody>
</table>
</div>


### METALLIC
<div style="overflow-x:auto;font-size:12px">
<table>
  <thead style="background-color:transparent">
    <tr style="border-color:transparent">
      <th>0.0</th><th>0.1</th><th>0.2</th><th>0.3</th><th>0.4</th><th>0.5</th><th>0.6</th><th>0.7</th><th>0.8</th><th>0.9</th><th>1.0</th>
    </tr>
  </thead>
  <tbody>
    <tr>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_00.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_01.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_02.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_03.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_04.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_05.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_06.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_07.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_08.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_09.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/metallic_10.png"></img></td>
    </tr>
  </tbody>
</table>
</div>

## REFLECTANCE/GRAYSCALE
Specular intensity for **non-metals**. The default is **0.5**, or **4%** reflectance.

<div style="overflow-x:auto;font-size:12px">
<table>
  <thead style="background-color:transparent">
    <tr style="border-color:transparent">
      <th>0.0</th><th>0.1</th><th>0.2</th><th>0.3</th><th>0.4</th><th>0.5</th><th>0.6</th><th>0.7</th><th>0.8</th><th>0.9</th><th>1.0</th>
    </tr>
  </thead>
  <tbody>
    <tr>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_00.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_01.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_02.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_03.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_04.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_05.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_06.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_07.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_08.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_09.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/reflectance_10.png"></img></td>
    </tr>
  </tbody>
</table>
</div>
<div style="display:flex;width:100%;flex-direction:column;font-size:12px;color:#aaa">
  <div style="display:flex;width:100%;flex-grow:1;margin-top:10px">
    <div style="display:flex;flex-direction:row;flex-grow:28;border-style:none;border-width:2px;border-radius:7px;border-color:#5a5a5a;text-align:center;padding:2px 0;align-items:center">
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
      <div style="display:flex;flex-direction:column">
        <div> No real-world material </div>
      </div>
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
    </div>
    <div style="display:flex">
        2%
    </div>
    <div style="display:flex;flex-direction:row;flex-grow:7;border-style:none;border-width:2px;border-radius:7px;border-color:#5a5a5a;text-align:center;padding:2px 0;align-items:center">
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
      <div style="display:flex;flex-direction:column">
        <div> Common dielectrics </div>
      </div>
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
    </div>
    <div style="display:flex">
        5%
    </div>
    <div style="display:flex;flex-direction:row;flex-grow:62;border-style:none;border-width:2px;border-radius:7px;border-color:#5a5a5a;text-align:center;padding:2px 0;align-items:center">
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
    </div>
    <div style="display:flex">
        16%
    </div>
  </div>
  <div style="align-self:end">
      Gemstones
  </div>
  <div style="display:flex;width:100%;flex-grow:1">
    <div style="flex-grow:35">&nbsp </div>
    <div style="display:flex;flex-direction:row;flex-grow:53;border-style:none;border-width:2px;border-radius:7px;border-color:#5a5a5a;text-align:center;padding:2px 0;align-items:center">
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
      <div style="display:flex;flex-direction:column">
        <div> All dielectrics </div>
      </div>
      <div style="display:flex;flex-grow:1;height:3px;background:#aaa;margin:0 5px"></div>
    </div>
  </div>
</div>


### SAMPLES
<div style="overflow-x:auto;font-size:12px">
<table>
  <tbody>
    <tr>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:rgba(90,90,90,1);">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:rgba(119,119,119,1);">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;flex-direction:row;width:100%;height:100%">
                <div style="background-color:rgb(90,90,90);flex-grow:1"></div>
                <div style="background-color:rgb(119,119,119);flex-grow:1"></div>
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:rgba(127,127,127,1);">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;flex-direction:row;width:100%;height:100%">
                <div style="background-color:rgb(90,90,90);flex-grow:1"></div>
                <div style="background-color:rgb(119,119,119);flex-grow:1"></div>
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:rgba(180,180,180,1);">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;width:100%;height:100%;background-color:rgba(255,255,255,1);">
            </div>
        </td>
        <td style="min-width:70px;height:42px;padding-left:0px;border-color:transparent;">
            <div style="display:flex;flex-direction:row;width:100%;height:100%">
                <div style="background-color:#8e8e8e;flex-grow:1"></div>
                <div style="background-color:white;flex-grow:1"></div>
            </div>
        </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> Water </td>
        <td style="border-color:transparent;padding:3px 10px"> Glass </td>
        <td style="border-color:transparent;padding:3px 10px"> Liquids </td>
        <td style="border-color:transparent;padding:3px 10px"> Defaults </td>
        <td style="border-color:transparent;padding:3px 10px"> Others </td>
        <td style="border-color:transparent;padding:3px 10px"> Ruby </td>
        <td style="border-color:transparent;padding:3px 10px"> Diamond </td>
        <td style="border-color:transparent;padding:3px 10px"> Gemstones </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> 90,90,90 </td>
        <td style="border-color:transparent;padding:3px 10px"> 119,119,119 </td>
        <td style="border-color:transparent;padding:3px 10px">  </td>
        <td style="border-color:transparent;padding:3px 10px"> 127,127,127 </td>
        <td style="border-color:transparent;padding:3px 10px">  </td>
        <td style="border-color:transparent;padding:3px 10px"> 180,180,180 </td>
        <td style="border-color:transparent;padding:3px 10px"> 255,255,255 </td>
        <td style="border-color:transparent;padding:3px 10px">  </td>
    </tr>
    <tr style="background-color:transparent">
        <td style="border-color:transparent;padding:3px 10px"> 2% </td>
        <td style="border-color:transparent;padding:3px 10px"> 3.5% </td>
        <td style="border-color:transparent;padding:3px 10px"> 2% to 4% </td>
        <td style="border-color:transparent;padding:3px 10px"> 4%</td>
        <td style="border-color:transparent;padding:3px 10px"> 2% to 5% </td>
        <td style="border-color:transparent;padding:3px 10px"> 8% </td>
        <td style="border-color:transparent;padding:3px 10px"> 16% </td>
        <td style="border-color:transparent;padding:3px 10px"> 5% to 16% </td>
    </tr>
  </tbody>
</table>
</div>

## CLEAR COAT/GRAYSCALE
Strength of the clear coat layer on top of a base **dielectric** or **conductor** layer.
The clear coat layer will commonly be set to **0.0** or **1.0**.
This layer has a fixed index of refraction of 1.5.

<div style="overflow-x:auto;font-size:12px">
<table>
  <thead style="background-color:transparent">
    <tr style="border-color:transparent">
      <th>0.0</th><th>0.1</th><th>0.2</th><th>0.3</th><th>0.4</th><th>0.5</th><th>0.6</th><th>0.7</th><th>0.8</th><th>0.9</th><th>1.0</th>
    </tr>
  </thead>
  <tbody>
    <tr>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_00.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_01.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_02.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_03.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_04.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_05.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_06.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_07.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_08.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_09.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_10.png"></img></td>
    </tr>
    <tr style="background-color:transparent">
      <td colspan="5" style="border-color:transparent">NO CLEAR COAT</td>
      <td colspan="6" style="border-color:transparent;text-align:right">FULL CLEAR COAT</td>
    </tr>
  </tbody>
</table>
</div>


## CLEAR COAT ROUGHNESS/GRAYSCALE
Defines the perceived **smoothness** (0.0) or **roughness** (1.0) of the clear coat layer.
It is sometimes called **glossiness**.
This may affect the roughness of the base layer.

<div style="overflow-x:auto;font-size:12px">
<table>
  <thead style="background-color:transparent">
    <tr style="border-color:transparent">
      <th>0.0</th><th>0.1</th><th>0.2</th><th>0.3</th><th>0.4</th><th>0.5</th><th>0.6</th><th>0.7</th><th>0.8</th><th>0.9</th><th>1.0</th>
    </tr>
  </thead>
  <tbody>
    <tr>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_00.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_01.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_02.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_03.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_04.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_05.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_06.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_07.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_08.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_09.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/clear_coat_roughness_10.png"></img></td>
    </tr>
    <tr style="background-color:transparent">
      <td colspan="5" style="border-color:transparent">GLOSSY CLEAR COAT</td>
      <td colspan="6" style="border-color:transparent;text-align:right">ROUGH CLEAR COAT</td>
    </tr>
  </tbody>
</table>
</div>


## ANISOTROPY/GRAYSCALE
Defines whether the material appearance is **directionally dependent**, that is **isotropic** (0.0)
or **anisotropic** (1.0). Brushed metals are **anisotropic**.
Values can be **negative** to change the orientation of the specular reflections.

<div style="overflow-x:auto;font-size:12px">
<table>
  <thead style="background-color:transparent">
    <tr style="border-color:transparent">
      <th>0.0</th><th>0.1</th><th>0.2</th><th>0.3</th><th>0.4</th><th>0.5</th><th>0.6</th><th>0.7</th><th>0.8</th><th>0.9</th><th>1.0</th>
    </tr>
  </thead>
  <tbody>
    <tr>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_00.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_01.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_02.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_03.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_04.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_05.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_06.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_07.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_08.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_09.png"></img></td>
        <td style="padding:0;border-color:transparent"><img style="min-width:40px" src="../images/anisotropy_10.png"></img></td>
    </tr>
    <tr style="background-color:transparent">
      <td colspan="5" style="border-color:transparent">ISOTROPIC</td>
      <td colspan="6" style="border-color:transparent;text-align:right">ANISOTROPIC</td>
    </tr>
  </tbody>
</table>
</div>
