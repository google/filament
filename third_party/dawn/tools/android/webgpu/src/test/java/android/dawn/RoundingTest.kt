package android.dawn

import android.dawn.helper.roundDownToNearestMultipleOf
import android.dawn.helper.roundUpToNearestMultipleOf
import org.junit.Assert
import org.junit.Test

class RoundingTest {

  @Test
  fun roundingIsCorrect() {
    Assert.assertEquals(1000, 1234.roundDownToNearestMultipleOf(1000));
    Assert.assertEquals(2000, 1234.roundUpToNearestMultipleOf(1000));

    Assert.assertEquals(-2000, (-1234).roundDownToNearestMultipleOf(1000));
    Assert.assertEquals(-1000, (-1234).roundUpToNearestMultipleOf(1000));

    // Int.MAX_VALUE is 2,147,483,647 (2^31 - 1)
    Assert.assertEquals(2_147_483_000L, 2_147_483_648L.roundDownToNearestMultipleOf(1000));
    Assert.assertEquals(2_147_484_000L, 2_147_483_648L.roundUpToNearestMultipleOf(1000));

    Assert.assertEquals(1000, 1000.roundDownToNearestMultipleOf(1000));
    Assert.assertEquals(1000, 1000.roundUpToNearestMultipleOf(1000));
    Assert.assertEquals(-1000, (-1000).roundDownToNearestMultipleOf(1000));
    Assert.assertEquals(-1000, (-1000).roundUpToNearestMultipleOf(1000));
  }
}