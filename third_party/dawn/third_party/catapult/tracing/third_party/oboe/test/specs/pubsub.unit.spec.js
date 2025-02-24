describe('pub sub', function(){

   it('is able to provide a single event pubsub', function(){
   
      var events = pubSub();
      
      expect(events('somethingHappening').on).not.toBeUndefined();
   
   });
   
   it('is provides the same single when asked twice', function(){
   
      var events = pubSub();
      
      expect(   events('somethingHappening'))
         .toBe( events('somethingHappening'));
   });
   
   it('numeric event names are treated as equal', function(){
   
      var events = pubSub();
      
      expect(   events(1))
         .toBe( events(1));      
   });
   
   it('different numeric event names are treated as equal', function(){
   
      var events = pubSub();
      
      expect(       events(1))
         .not.toBe( events(2));      
   });
   
   it('is provides different singles for different event names', function(){
   
      var events = pubSub();
      
      expect(        events('somethingHappening'))
         .not.toBe(  events('somethingElseHappening'));
   });      
      
   it('doesn\'t notify a newListener listener of its own addition', function() {
      var events = pubSub(),
          listenerListener = jasmine.createSpy('listenerListener');      

      events('newListener').on(listenerListener);
            
      expect(listenerListener).not.toHaveBeenCalled();   
   });   
   
   it('notifies of new listeners when added without an id', function() {
      var events = pubSub(),
          listenerListener = jasmine.createSpy('listenerListener');      

      events('newListener').on(listenerListener);
      events('foo').on(noop);
            
      expect(listenerListener).toHaveBeenCalledWith('foo', noop, noop);   
   });   
   
   it('should which multiple events notify of the right one', function(){
   
      var events = pubSub(),
          listenerA = jasmine.createSpy('listenerA'),
          listenerB = jasmine.createSpy('listenerB');
      
      events('eventA').on(listenerA);
      events('eventB').on(listenerB);
      
      events('eventA').emit();
      
      expect(listenerA).toHaveBeenCalled();
      expect(listenerB).not.toHaveBeenCalled();
      
      events('eventB').emit();
      
      expect(listenerB).toHaveBeenCalled();      
   });
   
     
   it('does not fire removeListener if nothing is removed', function() {
      
      var events                 = pubSub(),   
          removeListenerListener = jasmine.createSpy('removeListenerListener'),
          fooListener            = jasmine.createSpy('fooListener');
      
      events('removeListener').on(removeListenerListener);

      events('foo').on(fooListener);
      events('foo').un('wrong_item');
       
      expect(removeListenerListener).not.toHaveBeenCalled();         
   });
   
   it('fires removeListener when a listener is removed', function(){
      var events = pubSub(),   
          removeListenerListener = jasmine.createSpy('removeListenerListener');
   
      events('removeListener').on(removeListenerListener);
      
      events('foo').on(noop);
      events('foo').un(noop);
      
      expect(removeListenerListener).toHaveBeenCalledWith('foo', noop, noop);     
   })
   
   describe('short-cut methods', function(){
   
      it('has .emit', function(){
      
         var events     = pubSub(),
             callback   = jasmine.createSpy('something happening callback');
         
         events('somethingHappened').on(callback);
         events.emit('somethingHappened', 'it was', 'definitely something');
         
         expect(callback).toHaveBeenCalledWith('it was', 'definitely something');
      });
      
      it('has .on', function(){
      
         var events     = pubSub(),
             callback   = jasmine.createSpy('something happening callback');
         
         events.on('somethingHappened', callback);
         events('somethingHappened').emit('it was', 'definitely something');
         
         expect(callback).toHaveBeenCalledWith('it was', 'definitely something');
      });
      
      it('has .un', function(){
      
         var events     = pubSub(),
             callback   = jasmine.createSpy('something happening callback');
         
         events('somethingHappened').on(callback);
         events.un('somethingHappened', callback);
         events('somethingHappened').emit('it was', 'definitely something');                  
         
         expect(callback).not.toHaveBeenCalled();
      });
      
      it('has .un that works with listener id', function(){
      
         var events     = pubSub(),
             callback   = jasmine.createSpy('something happening callback');
         
         events('somethingHappened').on(callback, 'id');
         events.un('somethingHappened', 'id');
         events('somethingHappened').emit('it was', 'definitely something');                  
         
         expect(callback).not.toHaveBeenCalled();
      });            
   });   

});