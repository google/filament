/*
Copyright (c) 2013 Khaled Mammou - Advanced Micro Devices, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once
#ifndef O3DGC_VECTOR_INL
#define O3DGC_VECTOR_INL
namespace o3dgc
{
    template <typename T> 
    inline Vec3<T> operator*(T lhs, const Vec3<T> & rhs)
    {
        return Vec3<T>(lhs * rhs.X(), lhs * rhs.Y(), lhs * rhs.Z());
    }
    template <typename T>
    inline T & Vec3<T>::X()
    {
        return m_data[0];
    }
    template <typename T>
    inline  T &    Vec3<T>::Y()
    {
        return m_data[1];
    }
    template <typename T>
    inline  T &    Vec3<T>::Z()
    {
        return m_data[2];
    }
    template <typename T>
    inline  const T & Vec3<T>::X() const
    {
        return m_data[0];
    }
    template <typename T>
    inline  const T & Vec3<T>::Y() const
    {
        return m_data[1];
    }
    template <typename T>
    inline  const T & Vec3<T>::Z() const
    {
        return m_data[2];
    }
    template <typename T>
    inline  double Vec3<T>::GetNorm() const
    { 
        double a = (double) (m_data[0]);
        double b = (double) (m_data[1]);
        double c = (double) (m_data[2]);
        return sqrt(a*a+b*b+c*c);
    }
    template <typename T>
    inline  void Vec3<T>::operator= (const Vec3 & rhs)
    { 
        this->m_data[0] = rhs.m_data[0];
        this->m_data[1] = rhs.m_data[1];
        this->m_data[2] = rhs.m_data[2];
    }
    template <typename T>
    inline  void Vec3<T>::operator+=(const Vec3 & rhs)
    { 
        this->m_data[0] += rhs.m_data[0];
        this->m_data[1] += rhs.m_data[1];
        this->m_data[2] += rhs.m_data[2];
    }     
    template <typename T>
    inline void Vec3<T>::operator-=(const Vec3 & rhs)
    { 
        this->m_data[0] -= rhs.m_data[0];
        this->m_data[1] -= rhs.m_data[1];
        this->m_data[2] -= rhs.m_data[2];
    }
    template <typename T>
    inline void Vec3<T>::operator-=(T a)
    { 
        this->m_data[0] -= a;
        this->m_data[1] -= a;
        this->m_data[2] -= a;
    }
    template <typename T>
    inline void Vec3<T>::operator+=(T a)
    { 
        this->m_data[0] += a;
        this->m_data[1] += a;
        this->m_data[2] += a;
    }
    template <typename T>  
    inline void Vec3<T>::operator/=(T a)
    { 
        this->m_data[0] /= a;
        this->m_data[1] /= a;
        this->m_data[2] /= a;
    }
    template <typename T>  
    inline void Vec3<T>::operator*=(T a)
    { 
        this->m_data[0] *= a;
        this->m_data[1] *= a;
        this->m_data[2] *= a;
    }  
    template <typename T>
    inline Vec3<T> Vec3<T>::operator^ (const Vec3<T> & rhs) const
    {
        return Vec3<T>(m_data[1] * rhs.m_data[2] - m_data[2] * rhs.m_data[1],
                       m_data[2] * rhs.m_data[0] - m_data[0] * rhs.m_data[2],
                       m_data[0] * rhs.m_data[1] - m_data[1] * rhs.m_data[0]);
    }
    template <typename T>
    inline T Vec3<T>::operator*(const Vec3<T> & rhs) const
    {
        return (m_data[0] * rhs.m_data[0] + m_data[1] * rhs.m_data[1] + m_data[2] * rhs.m_data[2]);
    }        
    template <typename T>
    inline Vec3<T> Vec3<T>::operator+(const Vec3<T> & rhs) const
    {
        return Vec3<T>(m_data[0] + rhs.m_data[0],m_data[1] + rhs.m_data[1],m_data[2] + rhs.m_data[2]);
    }
    template <typename T> 
    inline  Vec3<T> Vec3<T>::operator-(const Vec3<T> & rhs) const
    {
        return Vec3<T>(m_data[0] - rhs.m_data[0],m_data[1] - rhs.m_data[1],m_data[2] - rhs.m_data[2]);
    }     
    template <typename T> 
    inline  Vec3<T> Vec3<T>::operator-() const
    {
        return Vec3<T>(-m_data[0],-m_data[1],-m_data[2]);
    }     

    template <typename T> 
    inline Vec3<T> Vec3<T>::operator*(T rhs) const
    {
        return Vec3<T>(rhs * this->m_data[0], rhs * this->m_data[1], rhs * this->m_data[2]);
    }
    template <typename T>
    inline Vec3<T> Vec3<T>::operator/ (T rhs) const
    {
        return Vec3<T>(m_data[0] / rhs, m_data[1] / rhs, m_data[2] / rhs);
    }
    template <typename T>
    inline Vec3<T>::Vec3(T a) 
    { 
        m_data[0] = m_data[1] = m_data[2] = a;
    }
    template <typename T>
    inline Vec3<T>::Vec3(T x, T y, T z)
    {
        m_data[0] = x;
        m_data[1] = y;
        m_data[2] = z;
    }
    template <typename T>
    inline Vec3<T>::Vec3(const Vec3 & rhs)
    {        
        m_data[0] = rhs.m_data[0];
        m_data[1] = rhs.m_data[1];
        m_data[2] = rhs.m_data[2];
    }
    template <typename T>
    inline Vec3<T>::~Vec3(void){};

    template <typename T>
    inline Vec3<T>::Vec3() {}
    
    template <typename T>
    inline Vec2<T> operator*(T lhs, const Vec2<T> & rhs)
    {
        return Vec2<T>(lhs * rhs.X(), lhs * rhs.Y());
    }
    template <typename T>
    inline T & Vec2<T>::X()
    {
        return m_data[0];
    }
    template <typename T>
    inline  T &    Vec2<T>::Y()
    {
        return m_data[1];
    }
    template <typename T>
    inline  const T & Vec2<T>::X() const
    {
        return m_data[0];
    }
    template <typename T>
    inline  const T & Vec2<T>::Y() const
    {
        return m_data[1];
    }
    template <typename T>
    inline  double Vec2<T>::GetNorm() const
    { 
        double a = (double) (m_data[0]);
        double b = (double) (m_data[1]);
        return sqrt(a*a+b*b);
    }
    template <typename T>
    inline  void Vec2<T>::operator= (const Vec2 & rhs)
    { 
        this->m_data[0] = rhs.m_data[0]; 
        this->m_data[1] = rhs.m_data[1]; 
    }
    template <typename T>
    inline  void Vec2<T>::operator+=(const Vec2 & rhs)
    { 
        this->m_data[0] += rhs.m_data[0];
        this->m_data[1] += rhs.m_data[1];
    }     
    template <typename T>  
    inline void Vec2<T>::operator-=(const Vec2 & rhs)
    { 
        this->m_data[0] -= rhs.m_data[0];
        this->m_data[1] -= rhs.m_data[1];
    }
    template <typename T>
    inline void Vec2<T>::operator-=(T a)
    { 
        this->m_data[0] -= a;
        this->m_data[1] -= a;
    }
    template <typename T>
    inline void Vec2<T>::operator+=(T a)
    { 
        this->m_data[0] += a;
        this->m_data[1] += a;
    }
    template <typename T>
    inline void Vec2<T>::operator/=(T a)
    { 
        this->m_data[0] /= a;
        this->m_data[1] /= a;
    }
    template <typename T>
    inline void Vec2<T>::operator*=(T a)
    { 
        this->m_data[0] *= a;
        this->m_data[1] *= a;
    }  
    template <typename T>
    inline T Vec2<T>::operator^ (const Vec2<T> & rhs) const
    {
        return m_data[0] * rhs.m_data[1] - m_data[1] * rhs.m_data[0];
    }
    template <typename T>
    inline T Vec2<T>::operator*(const Vec2<T> & rhs) const
    {
        return (m_data[0] * rhs.m_data[0] + m_data[1] * rhs.m_data[1]);
    }        
    template <typename T>
    inline Vec2<T> Vec2<T>::operator+(const Vec2<T> & rhs) const
    {
        return Vec2<T>(m_data[0] + rhs.m_data[0],m_data[1] + rhs.m_data[1]);
    }
    template <typename T>
    inline  Vec2<T> Vec2<T>::operator-(const Vec2<T> & rhs) const
    {
        return Vec2<T>(m_data[0] - rhs.m_data[0],m_data[1] - rhs.m_data[1]);
    }     
    template <typename T>
    inline  Vec2<T> Vec2<T>::operator-() const
    {
        return Vec2<T>(-m_data[0],-m_data[1]) ;
    }     

    template <typename T>
    inline Vec2<T> Vec2<T>::operator*(T rhs) const
    {
        return Vec2<T>(rhs * this->m_data[0], rhs * this->m_data[1]);
    }
    template <typename T>
    inline Vec2<T> Vec2<T>::operator/ (T rhs) const
    {
        return Vec2<T>(m_data[0] / rhs, m_data[1] / rhs);
    }
    template <typename T>
    inline Vec2<T>::Vec2(T a)
    { 
        m_data[0] = m_data[1] = a;
    }
    template <typename T>
    inline Vec2<T>::Vec2(T x, T y)
    {
        m_data[0] = x;
        m_data[1] = y;
    }
    template <typename T>
    inline Vec2<T>::Vec2(const Vec2 & rhs)
    {        
        m_data[0] = rhs.m_data[0];
        m_data[1] = rhs.m_data[1];
    }
    template <typename T>
    inline Vec2<T>::~Vec2(void){};

    template <typename T>
    inline Vec2<T>::Vec2() {}
}
#endif //O3DGC_VECTOR_INL

