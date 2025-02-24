'use strict';
class StateManager {
  constructor() {
    /** @private @const {Array<Object>} history_ The stack of old states. */
    this.history_ = [];
    /** @private @const {Object} current_ The currently displayed data. */
    this.current_ = undefined;
  }

  /**
   * Sets the supplied state as the currently displayed state
   * and pushes the previously displayed state onto the stack.
   * @param {Object} state
   */
  pushState(state) {
    if (this.current_ !== undefined) {
      this.history_.push(this.current_);
    }
    this.current_ = state;
  }

  /**
   * Sets the currently displayed state to be the state on the top
   * of the stack and returns this state.
   */
  popState() {
    this.current_ = this.history_.pop() || undefined;
    return this.current_;
  }

  /**
   * Checks if the stack has any state saved.
   * @returns {boolean} True iff the stack has state objects pushed.
   */
  hasHistory() {
    return this.history_.length > 0;
  }
}
