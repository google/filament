'use strict';
describe('stateManager', function() {
  const pushState = (sm, state) => {
    sm.pushState(state);
    return state;
  };
  describe('undo', function() {
    it('should return undefined when there are no items', function() {
      const sm = new StateManager();
      chai.expect(sm.popState()).to.be.undefined;
    });
    it('should return the items in correct order', function() {
      const sm = new StateManager();
      const stateOne = pushState(sm, {a: 1});
      const stateTwo = pushState(sm, {a: 2});
      pushState(sm, {a: 3});
      // The current state is state three ({a: 3}) so when an undo
      // is triggered it should go to the previous state,
      // state two.
      chai.expect(sm.popState()).to.eql(stateTwo);
      chai.expect(sm.popState()).to.eql(stateOne);
      chai.expect(sm.popState()).to.be.undefined;
    });
    it('should have nothing in the history stack when only one item is pushed',
        function() {
          const sm = new StateManager();
          pushState(sm, {});
          chai.expect(sm.popState()).to.be.undefined;
        });
  });
  describe('hasHistory', function() {
    it('should return true when there is history', function() {
      const sm = new StateManager();
      pushState(sm, {a: 1});
      pushState(sm, {a: 2});
      chai.expect(sm.hasHistory()).to.equal(true);
    });
    it('should return false when there is no history', function() {
      const sm = new StateManager();
      pushState(sm, {a: 1});
      chai.expect(sm.hasHistory()).to.equal(false);
    });
  });
});
