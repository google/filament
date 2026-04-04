package com.google.android.filament.validation

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.util.AttributeSet
import android.view.View

class TestProgressBar(context: Context, attrs: AttributeSet? = null) : View(context, attrs) {
    private var maxTests = 1
    private val results = mutableListOf<Boolean>()
    
    private val paintPass = Paint().apply { color = Color.parseColor("#4CAF50") } // Green
    private val paintFail = Paint().apply { color = Color.parseColor("#F44336") } // Red
    private val paintBg = Paint().apply { color = Color.parseColor("#E0E0E0") }   // Gray

    fun reset(max: Int) {
        maxTests = Math.max(1, max)
        results.clear()
        invalidate()
    }

    fun addResult(passed: Boolean) {
        results.add(passed)
        invalidate()
    }
    
    // Kept for partial compatibility with previous ProgressBar
    fun setMax(max: Int) {
        maxTests = Math.max(1, max)
        invalidate()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        
        val w = width.toFloat()
        val h = height.toFloat()
        
        // Draw background
        canvas.drawRect(0f, 0f, w, h, paintBg)
        
        if (results.isEmpty() || maxTests <= 0) return
        
        val segmentWidth = w / maxTests
        
        for (i in results.indices) {
            val paint = if (results[i]) paintPass else paintFail
            val left = i * segmentWidth
            val right = (i + 1) * segmentWidth
            canvas.drawRect(left, 0f, right, h, paint)
        }
    }
}
