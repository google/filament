# D3D11 multithread protected

Optional feature available on the D3D11 backend that when enabled makes all calls to the ID3D11DeviceContext happen inside a ID3D11Mulithread scope.
This ensure that the D3D11 device can be used concurrently by other components, in particular ANGLE in Chromium.

The initial tracking bug was crbug.com/dawn/1927.
