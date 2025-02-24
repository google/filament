fn a() {
  var a = 0;
  switch(a) {
    case 0, 2, 4: {
      var b = 3u;
      switch(b) {
        case 0: {
          break;
        }
        case 1, 2, 3, default: {
          var c = 123u;
          switch(c) {
            case 0: {
              break;
            }
            default: {
              return;
            }
          }
          return;
        }
      }
      break;
    }
    case 1, default: {
      return;
    }
  }
}
