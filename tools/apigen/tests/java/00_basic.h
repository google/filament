/**
 * A simple class to test the generator.
 *
 * Details about the class.
 */
class Foo {
public:
    /**
     * A simple method.
     *
     * Details about the trivialMethod.
     */
    void trivialMethod() noexcept;

    /**
     * A simple const method.
     *
     * Details about the constMethod.
     */
    void constMethod() const noexcept;

    /**
     * A method with parameters.
     *
     * Details about the methodWithParams.
     *
     * @param x The x parameter.
     * @param y The y parameter.
     * @see methodWithReturnValueAndParams
     */
    void methodWithParams(float x, float y) noexcept;

    /**
     * A method with a return value.
     *
     * Details about the methodWithReturnValue.
     *
     * @return The return value.
     * @see methodWithReturnValueAndParams
     */
    float methodWithReturnValue() noexcept;

    /** 
     * A method with a return value and parameters.
     *
     * Details about the methodWithReturnValueAndParams.
     *
     * @param x The x parameter.
     * @param y The y parameter.
     *
     * @return The return value.
     * @see methodWithReturnValue, methodWithParams
     */
    float methodWithReturnValueAndParams(float x, float y) noexcept;
};
