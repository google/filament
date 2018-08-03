Module['arguments'] = ['0'];
//Gamepads don't appear until a button is pressed and the joystick/gamepad tests expect one to be connected
Module['preRun'].push(function()
{
    Module['print']("Waiting for gamepad...");
    Module['addRunDependency']("gamepad");
    window.addEventListener('gamepadconnected', function()
    {
        //OK, got one
        Module['removeRunDependency']("gamepad");
    }, false);

    //chrome
    if(!!navigator.webkitGetGamepads)
    {
        var timeout = function()
        {
            if(navigator.webkitGetGamepads()[0] !== undefined)
                Module['removeRunDependency']("gamepad");
            else
                setTimeout(timeout, 100);
        }
        setTimeout(timeout, 100);
    }
});
