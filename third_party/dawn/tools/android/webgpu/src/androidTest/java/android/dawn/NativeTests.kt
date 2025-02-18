package android.dawn

import androidx.test.ext.junitgtest.GtestRunner;
import androidx.test.ext.junitgtest.TargetLibrary;
import org.junit.runner.RunWith;

/**
 * Runs the native tests. No implmentation is needed in Kotlin.
 * The tests are defined in the 'webgpu_wrapper_tests' native library.
 */
@RunWith(GtestRunner.class)
@TargetLibrary(libraryName = "webgpu_wrapper_tests")
public class NativeTests