#Examples of using Web Animations

Property indexed keyframes syntax
---------------------------------
- Each CSS property specifies its keyframe values as a list, different properties may have differently sized lists.
- The `easing` property applies its timing function to all keyframes.

[**Live demo**](http://jsbin.com/qiyeriruru/edit?js,output)
```javascript
element.animate({
  transform: [
    'scaleY(0.5)',
    'scaleX(0.5)',
    'scaleY(0.5)',
  ],
  background: [
    'red',
    'blue',
    'orange',
    'red',
  ],
  easing: 'ease-in-out',
}, {
  duration: 2000,
  iterations: Infinity,
});
```

Keyframe list syntax
--------------------
- Keyframes can be specified as a list of multiple CSS property values.
- Individual keyframes can be given specific offsets and easings.
- Not all properties need to be specified in every keyframe.
- Offsets are implicitly distributed if not specified.

[**Live demo**](http://jsbin.com/yajatoyere/edit?js,output)
```javascript
element.animate([
  {
    background: 'red',
    transform: 'none',
    easing: 'ease-out',
  },
  {
    offset: 0.1,
    transform: 'translateY(100px)',
    easing: 'ease-in-out',
  },
  {
    offset: 0.2,
    transform: 'translate(100px, 100px)',
    easing: 'ease-in-out',
  },
  {
    offset: 0.4,
    transform: 'translateX(100px)',
    easing: 'ease-out',
  },
  {
    background: 'blue',
    transform: 'none',
  },
], {
  duration: 4000,
  iterations: Infinity,
});
```

Timing parameters
-----------------
- Web Animations inherits many of its timing parameters from CSS Animations.
- See the [specification](http://w3c.github.io/web-animations/#animationeffecttimingreadonly) for details on each parameter.

[**Live demo**](http://jsbin.com/dabehipiyo/edit?js,output)
```javascript
element.animate({
  transform: ['none', 'translateX(100px)'],
  background: ['green', 'lime'],
}, {
  // Apply effect during delay.
  fill: 'backwards',

  // Delay starting by 500ms.
  delay: 500,

  // Iterations last for 2000ms.
  duration: 2000,

  // Start at 25% through an iteration.
  iterationStart: 0.25,

  // Run for 2 iterations.
  iterations: 2,

  // Play every second iteration backwards.
  direction: 'alternate',

  // Stop animating 500ms earlier.
  endDelay: -500,

  // The timing function to use with each iteration.
  easing: 'ease-in-out',
});
```

Playback controls
-----------------
- element.animate() returns an Animation object with basic playback controls.
- See the [specification](http://w3c.github.io/web-animations/#the-animation-interface) for details on each method.

[**Live demo**](http://jsbin.com/kutaqoxejo/edit?js,output)
```javascript
var animation = element.animate({
  transform: ['none', 'translateX(200px)'],
  background: ['red', 'orange'],
}, {
  duration: 4000,
  fill: 'both',
});
animation.play();
animation.reverse();
animation.pause();
animation.currentTime = 2000;
animation.playbackRate += 0.25;
animation.playbackRate -= 0.25;
animation.finish();
animation.cancel();
```

Transitioning states with element.animate()
-------------------------------------------
- This is an example of how to animate from one state to another using Web Animations.

[**Live demo**](http://jsbin.com/musufiwule/edit?js,output)
```javascript
var isOpen = false;
var openHeight = '100px';
var closedHeight = '0px';
var duration = 300;

button.addEventListener('click', function() {
  // Prevent clicks while we transition states.
  button.disabled = true;
  button.textContent = '...';

  // Determine where we're animation from/to.
  var fromHeight = isOpen ? openHeight : closedHeight;
  var toHeight = isOpen ? closedHeight : openHeight;

  // Start an animation transitioning from our current state to the final state.
  var animation = element.animate({ height: [fromHeight, toHeight] }, duration);

  // Update the button once the animation finishes.
  animation.onfinish = function() {
    isOpen = !isOpen;
    button.textContent = isOpen ? 'Close' : 'Open';
    button.disabled = false;
  };

  // Put our element in the final state.
  // Inline styles are overridden by active animations.
  // When the above animation finishes it will stop applying and
  // the element's style will fall back onto this inline style value.
  element.style.setProperty('height', toHeight);
});
```

Generating animations
---------------------
- The Javascript API allows for procedurally generating a diverse range of interesting animations.

[**Live demo**](http://jsbin.com/xolacasiyu/edit?js,output)
```html
<!DOCTYPE html>
<script src="https://rawgit.com/web-animations/web-animations-js/master/web-animations.min.js"></script>

<style>
#perspective {
  margin-left: 100px;
  width: 300px;
  height: 300px;
  perspective: 600px;
}

#container {
  width: 300px;
  height: 300px;
  line-height: 0;
  transform-style: preserve-3d;
}

.box {
  display: inline-block;
  width: 20px;
  height: 20px;
  background: black;
}
</style>

<div id="perspective">
  <div id="container"></div>
</div>

<script>
container.animate({
  transform: [
    'rotateX(70deg) rotateZ(0deg)',
    'rotateX(70deg) rotateZ(360deg)',
  ],
}, {
  duration: 20000,
  iterations: Infinity,
});

for (var y = -7; y <= 7; y++) {
  for (var x = -7; x <= 7; x++) {
    var box = createBox();
    box.animate({
      transform: [
        'translateZ(0px)',
        'translateZ(20px)',
      ],
      opacity: [1, 0],
    }, {
      delay: (x*x + y*y) * 20,
      duration: 2000,
      iterations: Infinity,
      direction: 'alternate',
      easing: 'ease-in',
    });
  }
}

function createBox() {
  var box = document.createElement('div');
  box.className = 'box';
  container.appendChild(box);
  return box;
}
</script>
```
