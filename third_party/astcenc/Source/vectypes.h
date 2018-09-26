/*----------------------------------------------------------------------------*/  
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Template library for fixed-size vectors.
 */ 
/*----------------------------------------------------------------------------*/ 

#include <string.h>
#include <stdint.h>

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned char uchar;
typedef signed char schar;

template < typename vtype > class vtype2;

template < typename vtype > class vtype3;

template < typename vtype > class vtype4;

template < typename vtype > struct vtype2_xx_ref
{
	vtype2 < vtype > *v;
	vtype2_xx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xy_ref
{
	vtype2 < vtype > *v;
	vtype2_xy_ref(vtype2 < vtype > *p):v(p)
	{
	};
	inline vtype2_xy_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype2_yx_ref
{
	vtype2 < vtype > *v;
	vtype2_yx_ref(vtype2 < vtype > *p):v(p)
	{
	};
	inline vtype2_yx_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype2_yy_ref
{
	vtype2 < vtype > *v;
	vtype2_yy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xx_ref
{
	vtype3 < vtype > *v;
	vtype3_xx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xy_ref
{
	vtype3 < vtype > *v;
	vtype3_xy_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_xy_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype3_xz_ref
{
	vtype3 < vtype > *v;
	vtype3_xz_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_xz_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype3_yx_ref
{
	vtype3 < vtype > *v;
	vtype3_yx_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_yx_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype3_yy_ref
{
	vtype3 < vtype > *v;
	vtype3_yy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yz_ref
{
	vtype3 < vtype > *v;
	vtype3_yz_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_yz_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype3_zx_ref
{
	vtype3 < vtype > *v;
	vtype3_zx_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_zx_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype3_zy_ref
{
	vtype3 < vtype > *v;
	vtype3_zy_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_zy_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype3_zz_ref
{
	vtype3 < vtype > *v;
	vtype3_zz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xx_ref
{
	vtype4 < vtype > *v;
	vtype4_xx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xy_ref
{
	vtype4 < vtype > *v;
	vtype4_xy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xy_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_xz_ref
{
	vtype4 < vtype > *v;
	vtype4_xz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xz_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_xw_ref
{
	vtype4 < vtype > *v;
	vtype4_xw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xw_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_yx_ref
{
	vtype4 < vtype > *v;
	vtype4_yx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yx_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_yy_ref
{
	vtype4 < vtype > *v;
	vtype4_yy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yz_ref
{
	vtype4 < vtype > *v;
	vtype4_yz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yz_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_yw_ref
{
	vtype4 < vtype > *v;
	vtype4_yw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yw_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_zx_ref
{
	vtype4 < vtype > *v;
	vtype4_zx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zx_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_zy_ref
{
	vtype4 < vtype > *v;
	vtype4_zy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zy_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_zz_ref
{
	vtype4 < vtype > *v;
	vtype4_zz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zw_ref
{
	vtype4 < vtype > *v;
	vtype4_zw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zw_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_wx_ref
{
	vtype4 < vtype > *v;
	vtype4_wx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wx_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_wy_ref
{
	vtype4 < vtype > *v;
	vtype4_wy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wy_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_wz_ref
{
	vtype4 < vtype > *v;
	vtype4_wz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wz_ref & operator=(const vtype2 < vtype > &);
};
template < typename vtype > struct vtype4_ww_ref
{
	vtype4 < vtype > *v;
	vtype4_ww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xxx_ref
{
	vtype2 < vtype > *v;
	vtype2_xxx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xxy_ref
{
	vtype2 < vtype > *v;
	vtype2_xxy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xyx_ref
{
	vtype2 < vtype > *v;
	vtype2_xyx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xyy_ref
{
	vtype2 < vtype > *v;
	vtype2_xyy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yxx_ref
{
	vtype2 < vtype > *v;
	vtype2_yxx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yxy_ref
{
	vtype2 < vtype > *v;
	vtype2_yxy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yyx_ref
{
	vtype2 < vtype > *v;
	vtype2_yyx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yyy_ref
{
	vtype2 < vtype > *v;
	vtype2_yyy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxx_ref
{
	vtype3 < vtype > *v;
	vtype3_xxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxy_ref
{
	vtype3 < vtype > *v;
	vtype3_xxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxz_ref
{
	vtype3 < vtype > *v;
	vtype3_xxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyx_ref
{
	vtype3 < vtype > *v;
	vtype3_xyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyy_ref
{
	vtype3 < vtype > *v;
	vtype3_xyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyz_ref
{
	vtype3 < vtype > *v;
	vtype3_xyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_xyz_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype3_xzx_ref
{
	vtype3 < vtype > *v;
	vtype3_xzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzy_ref
{
	vtype3 < vtype > *v;
	vtype3_xzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_xzy_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype3_xzz_ref
{
	vtype3 < vtype > *v;
	vtype3_xzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxx_ref
{
	vtype3 < vtype > *v;
	vtype3_yxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxy_ref
{
	vtype3 < vtype > *v;
	vtype3_yxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxz_ref
{
	vtype3 < vtype > *v;
	vtype3_yxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_yxz_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype3_yyx_ref
{
	vtype3 < vtype > *v;
	vtype3_yyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyy_ref
{
	vtype3 < vtype > *v;
	vtype3_yyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyz_ref
{
	vtype3 < vtype > *v;
	vtype3_yyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzx_ref
{
	vtype3 < vtype > *v;
	vtype3_yzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_yzx_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype3_yzy_ref
{
	vtype3 < vtype > *v;
	vtype3_yzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzz_ref
{
	vtype3 < vtype > *v;
	vtype3_yzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxx_ref
{
	vtype3 < vtype > *v;
	vtype3_zxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxy_ref
{
	vtype3 < vtype > *v;
	vtype3_zxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_zxy_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype3_zxz_ref
{
	vtype3 < vtype > *v;
	vtype3_zxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyx_ref
{
	vtype3 < vtype > *v;
	vtype3_zyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
	inline vtype3_zyx_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype3_zyy_ref
{
	vtype3 < vtype > *v;
	vtype3_zyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyz_ref
{
	vtype3 < vtype > *v;
	vtype3_zyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzx_ref
{
	vtype3 < vtype > *v;
	vtype3_zzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzy_ref
{
	vtype3 < vtype > *v;
	vtype3_zzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzz_ref
{
	vtype3 < vtype > *v;
	vtype3_zzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxx_ref
{
	vtype4 < vtype > *v;
	vtype4_xxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxy_ref
{
	vtype4 < vtype > *v;
	vtype4_xxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxz_ref
{
	vtype4 < vtype > *v;
	vtype4_xxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxw_ref
{
	vtype4 < vtype > *v;
	vtype4_xxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyx_ref
{
	vtype4 < vtype > *v;
	vtype4_xyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyy_ref
{
	vtype4 < vtype > *v;
	vtype4_xyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyz_ref
{
	vtype4 < vtype > *v;
	vtype4_xyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xyz_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_xyw_ref
{
	vtype4 < vtype > *v;
	vtype4_xyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xyw_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_xzx_ref
{
	vtype4 < vtype > *v;
	vtype4_xzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzy_ref
{
	vtype4 < vtype > *v;
	vtype4_xzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xzy_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_xzz_ref
{
	vtype4 < vtype > *v;
	vtype4_xzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzw_ref
{
	vtype4 < vtype > *v;
	vtype4_xzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xzw_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_xwx_ref
{
	vtype4 < vtype > *v;
	vtype4_xwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwy_ref
{
	vtype4 < vtype > *v;
	vtype4_xwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xwy_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_xwz_ref
{
	vtype4 < vtype > *v;
	vtype4_xwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xwz_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_xww_ref
{
	vtype4 < vtype > *v;
	vtype4_xww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxx_ref
{
	vtype4 < vtype > *v;
	vtype4_yxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxy_ref
{
	vtype4 < vtype > *v;
	vtype4_yxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxz_ref
{
	vtype4 < vtype > *v;
	vtype4_yxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yxz_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_yxw_ref
{
	vtype4 < vtype > *v;
	vtype4_yxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yxw_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_yyx_ref
{
	vtype4 < vtype > *v;
	vtype4_yyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyy_ref
{
	vtype4 < vtype > *v;
	vtype4_yyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyz_ref
{
	vtype4 < vtype > *v;
	vtype4_yyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyw_ref
{
	vtype4 < vtype > *v;
	vtype4_yyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzx_ref
{
	vtype4 < vtype > *v;
	vtype4_yzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yzx_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_yzy_ref
{
	vtype4 < vtype > *v;
	vtype4_yzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzz_ref
{
	vtype4 < vtype > *v;
	vtype4_yzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzw_ref
{
	vtype4 < vtype > *v;
	vtype4_yzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yzw_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_ywx_ref
{
	vtype4 < vtype > *v;
	vtype4_ywx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_ywx_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_ywy_ref
{
	vtype4 < vtype > *v;
	vtype4_ywy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywz_ref
{
	vtype4 < vtype > *v;
	vtype4_ywz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_ywz_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_yww_ref
{
	vtype4 < vtype > *v;
	vtype4_yww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxx_ref
{
	vtype4 < vtype > *v;
	vtype4_zxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxy_ref
{
	vtype4 < vtype > *v;
	vtype4_zxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zxy_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_zxz_ref
{
	vtype4 < vtype > *v;
	vtype4_zxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxw_ref
{
	vtype4 < vtype > *v;
	vtype4_zxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zxw_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_zyx_ref
{
	vtype4 < vtype > *v;
	vtype4_zyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zyx_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_zyy_ref
{
	vtype4 < vtype > *v;
	vtype4_zyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyz_ref
{
	vtype4 < vtype > *v;
	vtype4_zyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyw_ref
{
	vtype4 < vtype > *v;
	vtype4_zyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zyw_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_zzx_ref
{
	vtype4 < vtype > *v;
	vtype4_zzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzy_ref
{
	vtype4 < vtype > *v;
	vtype4_zzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzz_ref
{
	vtype4 < vtype > *v;
	vtype4_zzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzw_ref
{
	vtype4 < vtype > *v;
	vtype4_zzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwx_ref
{
	vtype4 < vtype > *v;
	vtype4_zwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zwx_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_zwy_ref
{
	vtype4 < vtype > *v;
	vtype4_zwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zwy_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_zwz_ref
{
	vtype4 < vtype > *v;
	vtype4_zwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zww_ref
{
	vtype4 < vtype > *v;
	vtype4_zww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxx_ref
{
	vtype4 < vtype > *v;
	vtype4_wxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxy_ref
{
	vtype4 < vtype > *v;
	vtype4_wxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wxy_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_wxz_ref
{
	vtype4 < vtype > *v;
	vtype4_wxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wxz_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_wxw_ref
{
	vtype4 < vtype > *v;
	vtype4_wxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyx_ref
{
	vtype4 < vtype > *v;
	vtype4_wyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wyx_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_wyy_ref
{
	vtype4 < vtype > *v;
	vtype4_wyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyz_ref
{
	vtype4 < vtype > *v;
	vtype4_wyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wyz_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_wyw_ref
{
	vtype4 < vtype > *v;
	vtype4_wyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzx_ref
{
	vtype4 < vtype > *v;
	vtype4_wzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wzx_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_wzy_ref
{
	vtype4 < vtype > *v;
	vtype4_wzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wzy_ref & operator=(const vtype3 < vtype > &);
};
template < typename vtype > struct vtype4_wzz_ref
{
	vtype4 < vtype > *v;
	vtype4_wzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzw_ref
{
	vtype4 < vtype > *v;
	vtype4_wzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwx_ref
{
	vtype4 < vtype > *v;
	vtype4_wwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwy_ref
{
	vtype4 < vtype > *v;
	vtype4_wwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwz_ref
{
	vtype4 < vtype > *v;
	vtype4_wwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_www_ref
{
	vtype4 < vtype > *v;
	vtype4_www_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xxxx_ref
{
	vtype2 < vtype > *v;
	vtype2_xxxx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xxxy_ref
{
	vtype2 < vtype > *v;
	vtype2_xxxy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xxyx_ref
{
	vtype2 < vtype > *v;
	vtype2_xxyx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xxyy_ref
{
	vtype2 < vtype > *v;
	vtype2_xxyy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xyxx_ref
{
	vtype2 < vtype > *v;
	vtype2_xyxx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xyxy_ref
{
	vtype2 < vtype > *v;
	vtype2_xyxy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xyyx_ref
{
	vtype2 < vtype > *v;
	vtype2_xyyx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_xyyy_ref
{
	vtype2 < vtype > *v;
	vtype2_xyyy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yxxx_ref
{
	vtype2 < vtype > *v;
	vtype2_yxxx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yxxy_ref
{
	vtype2 < vtype > *v;
	vtype2_yxxy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yxyx_ref
{
	vtype2 < vtype > *v;
	vtype2_yxyx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yxyy_ref
{
	vtype2 < vtype > *v;
	vtype2_yxyy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yyxx_ref
{
	vtype2 < vtype > *v;
	vtype2_yyxx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yyxy_ref
{
	vtype2 < vtype > *v;
	vtype2_yyxy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yyyx_ref
{
	vtype2 < vtype > *v;
	vtype2_yyyx_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype2_yyyy_ref
{
	vtype2 < vtype > *v;
	vtype2_yyyy_ref(vtype2 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxxx_ref
{
	vtype3 < vtype > *v;
	vtype3_xxxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxxy_ref
{
	vtype3 < vtype > *v;
	vtype3_xxxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxxz_ref
{
	vtype3 < vtype > *v;
	vtype3_xxxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxyx_ref
{
	vtype3 < vtype > *v;
	vtype3_xxyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxyy_ref
{
	vtype3 < vtype > *v;
	vtype3_xxyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxyz_ref
{
	vtype3 < vtype > *v;
	vtype3_xxyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxzx_ref
{
	vtype3 < vtype > *v;
	vtype3_xxzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxzy_ref
{
	vtype3 < vtype > *v;
	vtype3_xxzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xxzz_ref
{
	vtype3 < vtype > *v;
	vtype3_xxzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyxx_ref
{
	vtype3 < vtype > *v;
	vtype3_xyxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyxy_ref
{
	vtype3 < vtype > *v;
	vtype3_xyxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyxz_ref
{
	vtype3 < vtype > *v;
	vtype3_xyxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyyx_ref
{
	vtype3 < vtype > *v;
	vtype3_xyyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyyy_ref
{
	vtype3 < vtype > *v;
	vtype3_xyyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyyz_ref
{
	vtype3 < vtype > *v;
	vtype3_xyyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyzx_ref
{
	vtype3 < vtype > *v;
	vtype3_xyzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyzy_ref
{
	vtype3 < vtype > *v;
	vtype3_xyzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xyzz_ref
{
	vtype3 < vtype > *v;
	vtype3_xyzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzxx_ref
{
	vtype3 < vtype > *v;
	vtype3_xzxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzxy_ref
{
	vtype3 < vtype > *v;
	vtype3_xzxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzxz_ref
{
	vtype3 < vtype > *v;
	vtype3_xzxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzyx_ref
{
	vtype3 < vtype > *v;
	vtype3_xzyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzyy_ref
{
	vtype3 < vtype > *v;
	vtype3_xzyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzyz_ref
{
	vtype3 < vtype > *v;
	vtype3_xzyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzzx_ref
{
	vtype3 < vtype > *v;
	vtype3_xzzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzzy_ref
{
	vtype3 < vtype > *v;
	vtype3_xzzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_xzzz_ref
{
	vtype3 < vtype > *v;
	vtype3_xzzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxxx_ref
{
	vtype3 < vtype > *v;
	vtype3_yxxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxxy_ref
{
	vtype3 < vtype > *v;
	vtype3_yxxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxxz_ref
{
	vtype3 < vtype > *v;
	vtype3_yxxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxyx_ref
{
	vtype3 < vtype > *v;
	vtype3_yxyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxyy_ref
{
	vtype3 < vtype > *v;
	vtype3_yxyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxyz_ref
{
	vtype3 < vtype > *v;
	vtype3_yxyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxzx_ref
{
	vtype3 < vtype > *v;
	vtype3_yxzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxzy_ref
{
	vtype3 < vtype > *v;
	vtype3_yxzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yxzz_ref
{
	vtype3 < vtype > *v;
	vtype3_yxzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyxx_ref
{
	vtype3 < vtype > *v;
	vtype3_yyxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyxy_ref
{
	vtype3 < vtype > *v;
	vtype3_yyxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyxz_ref
{
	vtype3 < vtype > *v;
	vtype3_yyxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyyx_ref
{
	vtype3 < vtype > *v;
	vtype3_yyyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyyy_ref
{
	vtype3 < vtype > *v;
	vtype3_yyyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyyz_ref
{
	vtype3 < vtype > *v;
	vtype3_yyyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyzx_ref
{
	vtype3 < vtype > *v;
	vtype3_yyzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyzy_ref
{
	vtype3 < vtype > *v;
	vtype3_yyzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yyzz_ref
{
	vtype3 < vtype > *v;
	vtype3_yyzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzxx_ref
{
	vtype3 < vtype > *v;
	vtype3_yzxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzxy_ref
{
	vtype3 < vtype > *v;
	vtype3_yzxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzxz_ref
{
	vtype3 < vtype > *v;
	vtype3_yzxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzyx_ref
{
	vtype3 < vtype > *v;
	vtype3_yzyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzyy_ref
{
	vtype3 < vtype > *v;
	vtype3_yzyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzyz_ref
{
	vtype3 < vtype > *v;
	vtype3_yzyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzzx_ref
{
	vtype3 < vtype > *v;
	vtype3_yzzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzzy_ref
{
	vtype3 < vtype > *v;
	vtype3_yzzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_yzzz_ref
{
	vtype3 < vtype > *v;
	vtype3_yzzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxxx_ref
{
	vtype3 < vtype > *v;
	vtype3_zxxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxxy_ref
{
	vtype3 < vtype > *v;
	vtype3_zxxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxxz_ref
{
	vtype3 < vtype > *v;
	vtype3_zxxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxyx_ref
{
	vtype3 < vtype > *v;
	vtype3_zxyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxyy_ref
{
	vtype3 < vtype > *v;
	vtype3_zxyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxyz_ref
{
	vtype3 < vtype > *v;
	vtype3_zxyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxzx_ref
{
	vtype3 < vtype > *v;
	vtype3_zxzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxzy_ref
{
	vtype3 < vtype > *v;
	vtype3_zxzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zxzz_ref
{
	vtype3 < vtype > *v;
	vtype3_zxzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyxx_ref
{
	vtype3 < vtype > *v;
	vtype3_zyxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyxy_ref
{
	vtype3 < vtype > *v;
	vtype3_zyxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyxz_ref
{
	vtype3 < vtype > *v;
	vtype3_zyxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyyx_ref
{
	vtype3 < vtype > *v;
	vtype3_zyyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyyy_ref
{
	vtype3 < vtype > *v;
	vtype3_zyyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyyz_ref
{
	vtype3 < vtype > *v;
	vtype3_zyyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyzx_ref
{
	vtype3 < vtype > *v;
	vtype3_zyzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyzy_ref
{
	vtype3 < vtype > *v;
	vtype3_zyzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zyzz_ref
{
	vtype3 < vtype > *v;
	vtype3_zyzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzxx_ref
{
	vtype3 < vtype > *v;
	vtype3_zzxx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzxy_ref
{
	vtype3 < vtype > *v;
	vtype3_zzxy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzxz_ref
{
	vtype3 < vtype > *v;
	vtype3_zzxz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzyx_ref
{
	vtype3 < vtype > *v;
	vtype3_zzyx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzyy_ref
{
	vtype3 < vtype > *v;
	vtype3_zzyy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzyz_ref
{
	vtype3 < vtype > *v;
	vtype3_zzyz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzzx_ref
{
	vtype3 < vtype > *v;
	vtype3_zzzx_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzzy_ref
{
	vtype3 < vtype > *v;
	vtype3_zzzy_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype3_zzzz_ref
{
	vtype3 < vtype > *v;
	vtype3_zzzz_ref(vtype3 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxxx_ref
{
	vtype4 < vtype > *v;
	vtype4_xxxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxxy_ref
{
	vtype4 < vtype > *v;
	vtype4_xxxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxxz_ref
{
	vtype4 < vtype > *v;
	vtype4_xxxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxxw_ref
{
	vtype4 < vtype > *v;
	vtype4_xxxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxyx_ref
{
	vtype4 < vtype > *v;
	vtype4_xxyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxyy_ref
{
	vtype4 < vtype > *v;
	vtype4_xxyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxyz_ref
{
	vtype4 < vtype > *v;
	vtype4_xxyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxyw_ref
{
	vtype4 < vtype > *v;
	vtype4_xxyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxzx_ref
{
	vtype4 < vtype > *v;
	vtype4_xxzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxzy_ref
{
	vtype4 < vtype > *v;
	vtype4_xxzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxzz_ref
{
	vtype4 < vtype > *v;
	vtype4_xxzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxzw_ref
{
	vtype4 < vtype > *v;
	vtype4_xxzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxwx_ref
{
	vtype4 < vtype > *v;
	vtype4_xxwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxwy_ref
{
	vtype4 < vtype > *v;
	vtype4_xxwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxwz_ref
{
	vtype4 < vtype > *v;
	vtype4_xxwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xxww_ref
{
	vtype4 < vtype > *v;
	vtype4_xxww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyxx_ref
{
	vtype4 < vtype > *v;
	vtype4_xyxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyxy_ref
{
	vtype4 < vtype > *v;
	vtype4_xyxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyxz_ref
{
	vtype4 < vtype > *v;
	vtype4_xyxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyxw_ref
{
	vtype4 < vtype > *v;
	vtype4_xyxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyyx_ref
{
	vtype4 < vtype > *v;
	vtype4_xyyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyyy_ref
{
	vtype4 < vtype > *v;
	vtype4_xyyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyyz_ref
{
	vtype4 < vtype > *v;
	vtype4_xyyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyyw_ref
{
	vtype4 < vtype > *v;
	vtype4_xyyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyzx_ref
{
	vtype4 < vtype > *v;
	vtype4_xyzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyzy_ref
{
	vtype4 < vtype > *v;
	vtype4_xyzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyzz_ref
{
	vtype4 < vtype > *v;
	vtype4_xyzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xyzw_ref
{
	vtype4 < vtype > *v;
	vtype4_xyzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xyzw_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_xywx_ref
{
	vtype4 < vtype > *v;
	vtype4_xywx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xywy_ref
{
	vtype4 < vtype > *v;
	vtype4_xywy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xywz_ref
{
	vtype4 < vtype > *v;
	vtype4_xywz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xywz_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_xyww_ref
{
	vtype4 < vtype > *v;
	vtype4_xyww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzxx_ref
{
	vtype4 < vtype > *v;
	vtype4_xzxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzxy_ref
{
	vtype4 < vtype > *v;
	vtype4_xzxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzxz_ref
{
	vtype4 < vtype > *v;
	vtype4_xzxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzxw_ref
{
	vtype4 < vtype > *v;
	vtype4_xzxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzyx_ref
{
	vtype4 < vtype > *v;
	vtype4_xzyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzyy_ref
{
	vtype4 < vtype > *v;
	vtype4_xzyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzyz_ref
{
	vtype4 < vtype > *v;
	vtype4_xzyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzyw_ref
{
	vtype4 < vtype > *v;
	vtype4_xzyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xzyw_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_xzzx_ref
{
	vtype4 < vtype > *v;
	vtype4_xzzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzzy_ref
{
	vtype4 < vtype > *v;
	vtype4_xzzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzzz_ref
{
	vtype4 < vtype > *v;
	vtype4_xzzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzzw_ref
{
	vtype4 < vtype > *v;
	vtype4_xzzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzwx_ref
{
	vtype4 < vtype > *v;
	vtype4_xzwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzwy_ref
{
	vtype4 < vtype > *v;
	vtype4_xzwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xzwy_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_xzwz_ref
{
	vtype4 < vtype > *v;
	vtype4_xzwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xzww_ref
{
	vtype4 < vtype > *v;
	vtype4_xzww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwxx_ref
{
	vtype4 < vtype > *v;
	vtype4_xwxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwxy_ref
{
	vtype4 < vtype > *v;
	vtype4_xwxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwxz_ref
{
	vtype4 < vtype > *v;
	vtype4_xwxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwxw_ref
{
	vtype4 < vtype > *v;
	vtype4_xwxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwyx_ref
{
	vtype4 < vtype > *v;
	vtype4_xwyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwyy_ref
{
	vtype4 < vtype > *v;
	vtype4_xwyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwyz_ref
{
	vtype4 < vtype > *v;
	vtype4_xwyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xwyz_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_xwyw_ref
{
	vtype4 < vtype > *v;
	vtype4_xwyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwzx_ref
{
	vtype4 < vtype > *v;
	vtype4_xwzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwzy_ref
{
	vtype4 < vtype > *v;
	vtype4_xwzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_xwzy_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_xwzz_ref
{
	vtype4 < vtype > *v;
	vtype4_xwzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwzw_ref
{
	vtype4 < vtype > *v;
	vtype4_xwzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwwx_ref
{
	vtype4 < vtype > *v;
	vtype4_xwwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwwy_ref
{
	vtype4 < vtype > *v;
	vtype4_xwwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwwz_ref
{
	vtype4 < vtype > *v;
	vtype4_xwwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_xwww_ref
{
	vtype4 < vtype > *v;
	vtype4_xwww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxxx_ref
{
	vtype4 < vtype > *v;
	vtype4_yxxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxxy_ref
{
	vtype4 < vtype > *v;
	vtype4_yxxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxxz_ref
{
	vtype4 < vtype > *v;
	vtype4_yxxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxxw_ref
{
	vtype4 < vtype > *v;
	vtype4_yxxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxyx_ref
{
	vtype4 < vtype > *v;
	vtype4_yxyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxyy_ref
{
	vtype4 < vtype > *v;
	vtype4_yxyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxyz_ref
{
	vtype4 < vtype > *v;
	vtype4_yxyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxyw_ref
{
	vtype4 < vtype > *v;
	vtype4_yxyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxzx_ref
{
	vtype4 < vtype > *v;
	vtype4_yxzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxzy_ref
{
	vtype4 < vtype > *v;
	vtype4_yxzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxzz_ref
{
	vtype4 < vtype > *v;
	vtype4_yxzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxzw_ref
{
	vtype4 < vtype > *v;
	vtype4_yxzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yxzw_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_yxwx_ref
{
	vtype4 < vtype > *v;
	vtype4_yxwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxwy_ref
{
	vtype4 < vtype > *v;
	vtype4_yxwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yxwz_ref
{
	vtype4 < vtype > *v;
	vtype4_yxwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yxwz_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_yxww_ref
{
	vtype4 < vtype > *v;
	vtype4_yxww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyxx_ref
{
	vtype4 < vtype > *v;
	vtype4_yyxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyxy_ref
{
	vtype4 < vtype > *v;
	vtype4_yyxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyxz_ref
{
	vtype4 < vtype > *v;
	vtype4_yyxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyxw_ref
{
	vtype4 < vtype > *v;
	vtype4_yyxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyyx_ref
{
	vtype4 < vtype > *v;
	vtype4_yyyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyyy_ref
{
	vtype4 < vtype > *v;
	vtype4_yyyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyyz_ref
{
	vtype4 < vtype > *v;
	vtype4_yyyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyyw_ref
{
	vtype4 < vtype > *v;
	vtype4_yyyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyzx_ref
{
	vtype4 < vtype > *v;
	vtype4_yyzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyzy_ref
{
	vtype4 < vtype > *v;
	vtype4_yyzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyzz_ref
{
	vtype4 < vtype > *v;
	vtype4_yyzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyzw_ref
{
	vtype4 < vtype > *v;
	vtype4_yyzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yywx_ref
{
	vtype4 < vtype > *v;
	vtype4_yywx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yywy_ref
{
	vtype4 < vtype > *v;
	vtype4_yywy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yywz_ref
{
	vtype4 < vtype > *v;
	vtype4_yywz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yyww_ref
{
	vtype4 < vtype > *v;
	vtype4_yyww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzxx_ref
{
	vtype4 < vtype > *v;
	vtype4_yzxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzxy_ref
{
	vtype4 < vtype > *v;
	vtype4_yzxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzxz_ref
{
	vtype4 < vtype > *v;
	vtype4_yzxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzxw_ref
{
	vtype4 < vtype > *v;
	vtype4_yzxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yzxw_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_yzyx_ref
{
	vtype4 < vtype > *v;
	vtype4_yzyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzyy_ref
{
	vtype4 < vtype > *v;
	vtype4_yzyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzyz_ref
{
	vtype4 < vtype > *v;
	vtype4_yzyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzyw_ref
{
	vtype4 < vtype > *v;
	vtype4_yzyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzzx_ref
{
	vtype4 < vtype > *v;
	vtype4_yzzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzzy_ref
{
	vtype4 < vtype > *v;
	vtype4_yzzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzzz_ref
{
	vtype4 < vtype > *v;
	vtype4_yzzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzzw_ref
{
	vtype4 < vtype > *v;
	vtype4_yzzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzwx_ref
{
	vtype4 < vtype > *v;
	vtype4_yzwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_yzwx_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_yzwy_ref
{
	vtype4 < vtype > *v;
	vtype4_yzwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzwz_ref
{
	vtype4 < vtype > *v;
	vtype4_yzwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_yzww_ref
{
	vtype4 < vtype > *v;
	vtype4_yzww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywxx_ref
{
	vtype4 < vtype > *v;
	vtype4_ywxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywxy_ref
{
	vtype4 < vtype > *v;
	vtype4_ywxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywxz_ref
{
	vtype4 < vtype > *v;
	vtype4_ywxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_ywxz_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_ywxw_ref
{
	vtype4 < vtype > *v;
	vtype4_ywxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywyx_ref
{
	vtype4 < vtype > *v;
	vtype4_ywyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywyy_ref
{
	vtype4 < vtype > *v;
	vtype4_ywyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywyz_ref
{
	vtype4 < vtype > *v;
	vtype4_ywyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywyw_ref
{
	vtype4 < vtype > *v;
	vtype4_ywyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywzx_ref
{
	vtype4 < vtype > *v;
	vtype4_ywzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_ywzx_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_ywzy_ref
{
	vtype4 < vtype > *v;
	vtype4_ywzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywzz_ref
{
	vtype4 < vtype > *v;
	vtype4_ywzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywzw_ref
{
	vtype4 < vtype > *v;
	vtype4_ywzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywwx_ref
{
	vtype4 < vtype > *v;
	vtype4_ywwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywwy_ref
{
	vtype4 < vtype > *v;
	vtype4_ywwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywwz_ref
{
	vtype4 < vtype > *v;
	vtype4_ywwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_ywww_ref
{
	vtype4 < vtype > *v;
	vtype4_ywww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxxx_ref
{
	vtype4 < vtype > *v;
	vtype4_zxxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxxy_ref
{
	vtype4 < vtype > *v;
	vtype4_zxxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxxz_ref
{
	vtype4 < vtype > *v;
	vtype4_zxxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxxw_ref
{
	vtype4 < vtype > *v;
	vtype4_zxxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxyx_ref
{
	vtype4 < vtype > *v;
	vtype4_zxyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxyy_ref
{
	vtype4 < vtype > *v;
	vtype4_zxyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxyz_ref
{
	vtype4 < vtype > *v;
	vtype4_zxyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxyw_ref
{
	vtype4 < vtype > *v;
	vtype4_zxyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zxyw_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_zxzx_ref
{
	vtype4 < vtype > *v;
	vtype4_zxzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxzy_ref
{
	vtype4 < vtype > *v;
	vtype4_zxzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxzz_ref
{
	vtype4 < vtype > *v;
	vtype4_zxzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxzw_ref
{
	vtype4 < vtype > *v;
	vtype4_zxzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxwx_ref
{
	vtype4 < vtype > *v;
	vtype4_zxwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxwy_ref
{
	vtype4 < vtype > *v;
	vtype4_zxwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zxwy_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_zxwz_ref
{
	vtype4 < vtype > *v;
	vtype4_zxwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zxww_ref
{
	vtype4 < vtype > *v;
	vtype4_zxww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyxx_ref
{
	vtype4 < vtype > *v;
	vtype4_zyxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyxy_ref
{
	vtype4 < vtype > *v;
	vtype4_zyxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyxz_ref
{
	vtype4 < vtype > *v;
	vtype4_zyxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyxw_ref
{
	vtype4 < vtype > *v;
	vtype4_zyxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zyxw_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_zyyx_ref
{
	vtype4 < vtype > *v;
	vtype4_zyyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyyy_ref
{
	vtype4 < vtype > *v;
	vtype4_zyyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyyz_ref
{
	vtype4 < vtype > *v;
	vtype4_zyyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyyw_ref
{
	vtype4 < vtype > *v;
	vtype4_zyyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyzx_ref
{
	vtype4 < vtype > *v;
	vtype4_zyzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyzy_ref
{
	vtype4 < vtype > *v;
	vtype4_zyzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyzz_ref
{
	vtype4 < vtype > *v;
	vtype4_zyzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyzw_ref
{
	vtype4 < vtype > *v;
	vtype4_zyzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zywx_ref
{
	vtype4 < vtype > *v;
	vtype4_zywx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zywx_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_zywy_ref
{
	vtype4 < vtype > *v;
	vtype4_zywy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zywz_ref
{
	vtype4 < vtype > *v;
	vtype4_zywz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zyww_ref
{
	vtype4 < vtype > *v;
	vtype4_zyww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzxx_ref
{
	vtype4 < vtype > *v;
	vtype4_zzxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzxy_ref
{
	vtype4 < vtype > *v;
	vtype4_zzxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzxz_ref
{
	vtype4 < vtype > *v;
	vtype4_zzxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzxw_ref
{
	vtype4 < vtype > *v;
	vtype4_zzxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzyx_ref
{
	vtype4 < vtype > *v;
	vtype4_zzyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzyy_ref
{
	vtype4 < vtype > *v;
	vtype4_zzyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzyz_ref
{
	vtype4 < vtype > *v;
	vtype4_zzyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzyw_ref
{
	vtype4 < vtype > *v;
	vtype4_zzyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzzx_ref
{
	vtype4 < vtype > *v;
	vtype4_zzzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzzy_ref
{
	vtype4 < vtype > *v;
	vtype4_zzzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzzz_ref
{
	vtype4 < vtype > *v;
	vtype4_zzzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzzw_ref
{
	vtype4 < vtype > *v;
	vtype4_zzzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzwx_ref
{
	vtype4 < vtype > *v;
	vtype4_zzwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzwy_ref
{
	vtype4 < vtype > *v;
	vtype4_zzwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzwz_ref
{
	vtype4 < vtype > *v;
	vtype4_zzwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zzww_ref
{
	vtype4 < vtype > *v;
	vtype4_zzww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwxx_ref
{
	vtype4 < vtype > *v;
	vtype4_zwxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwxy_ref
{
	vtype4 < vtype > *v;
	vtype4_zwxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zwxy_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_zwxz_ref
{
	vtype4 < vtype > *v;
	vtype4_zwxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwxw_ref
{
	vtype4 < vtype > *v;
	vtype4_zwxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwyx_ref
{
	vtype4 < vtype > *v;
	vtype4_zwyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_zwyx_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_zwyy_ref
{
	vtype4 < vtype > *v;
	vtype4_zwyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwyz_ref
{
	vtype4 < vtype > *v;
	vtype4_zwyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwyw_ref
{
	vtype4 < vtype > *v;
	vtype4_zwyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwzx_ref
{
	vtype4 < vtype > *v;
	vtype4_zwzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwzy_ref
{
	vtype4 < vtype > *v;
	vtype4_zwzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwzz_ref
{
	vtype4 < vtype > *v;
	vtype4_zwzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwzw_ref
{
	vtype4 < vtype > *v;
	vtype4_zwzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwwx_ref
{
	vtype4 < vtype > *v;
	vtype4_zwwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwwy_ref
{
	vtype4 < vtype > *v;
	vtype4_zwwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwwz_ref
{
	vtype4 < vtype > *v;
	vtype4_zwwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_zwww_ref
{
	vtype4 < vtype > *v;
	vtype4_zwww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxxx_ref
{
	vtype4 < vtype > *v;
	vtype4_wxxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxxy_ref
{
	vtype4 < vtype > *v;
	vtype4_wxxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxxz_ref
{
	vtype4 < vtype > *v;
	vtype4_wxxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxxw_ref
{
	vtype4 < vtype > *v;
	vtype4_wxxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxyx_ref
{
	vtype4 < vtype > *v;
	vtype4_wxyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxyy_ref
{
	vtype4 < vtype > *v;
	vtype4_wxyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxyz_ref
{
	vtype4 < vtype > *v;
	vtype4_wxyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wxyz_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_wxyw_ref
{
	vtype4 < vtype > *v;
	vtype4_wxyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxzx_ref
{
	vtype4 < vtype > *v;
	vtype4_wxzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxzy_ref
{
	vtype4 < vtype > *v;
	vtype4_wxzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wxzy_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_wxzz_ref
{
	vtype4 < vtype > *v;
	vtype4_wxzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxzw_ref
{
	vtype4 < vtype > *v;
	vtype4_wxzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxwx_ref
{
	vtype4 < vtype > *v;
	vtype4_wxwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxwy_ref
{
	vtype4 < vtype > *v;
	vtype4_wxwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxwz_ref
{
	vtype4 < vtype > *v;
	vtype4_wxwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wxww_ref
{
	vtype4 < vtype > *v;
	vtype4_wxww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyxx_ref
{
	vtype4 < vtype > *v;
	vtype4_wyxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyxy_ref
{
	vtype4 < vtype > *v;
	vtype4_wyxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyxz_ref
{
	vtype4 < vtype > *v;
	vtype4_wyxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wyxz_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_wyxw_ref
{
	vtype4 < vtype > *v;
	vtype4_wyxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyyx_ref
{
	vtype4 < vtype > *v;
	vtype4_wyyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyyy_ref
{
	vtype4 < vtype > *v;
	vtype4_wyyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyyz_ref
{
	vtype4 < vtype > *v;
	vtype4_wyyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyyw_ref
{
	vtype4 < vtype > *v;
	vtype4_wyyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyzx_ref
{
	vtype4 < vtype > *v;
	vtype4_wyzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wyzx_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_wyzy_ref
{
	vtype4 < vtype > *v;
	vtype4_wyzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyzz_ref
{
	vtype4 < vtype > *v;
	vtype4_wyzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyzw_ref
{
	vtype4 < vtype > *v;
	vtype4_wyzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wywx_ref
{
	vtype4 < vtype > *v;
	vtype4_wywx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wywy_ref
{
	vtype4 < vtype > *v;
	vtype4_wywy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wywz_ref
{
	vtype4 < vtype > *v;
	vtype4_wywz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wyww_ref
{
	vtype4 < vtype > *v;
	vtype4_wyww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzxx_ref
{
	vtype4 < vtype > *v;
	vtype4_wzxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzxy_ref
{
	vtype4 < vtype > *v;
	vtype4_wzxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wzxy_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_wzxz_ref
{
	vtype4 < vtype > *v;
	vtype4_wzxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzxw_ref
{
	vtype4 < vtype > *v;
	vtype4_wzxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzyx_ref
{
	vtype4 < vtype > *v;
	vtype4_wzyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
	inline vtype4_wzyx_ref & operator=(const vtype4 < vtype > &);
};
template < typename vtype > struct vtype4_wzyy_ref
{
	vtype4 < vtype > *v;
	vtype4_wzyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzyz_ref
{
	vtype4 < vtype > *v;
	vtype4_wzyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzyw_ref
{
	vtype4 < vtype > *v;
	vtype4_wzyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzzx_ref
{
	vtype4 < vtype > *v;
	vtype4_wzzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzzy_ref
{
	vtype4 < vtype > *v;
	vtype4_wzzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzzz_ref
{
	vtype4 < vtype > *v;
	vtype4_wzzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzzw_ref
{
	vtype4 < vtype > *v;
	vtype4_wzzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzwx_ref
{
	vtype4 < vtype > *v;
	vtype4_wzwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzwy_ref
{
	vtype4 < vtype > *v;
	vtype4_wzwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzwz_ref
{
	vtype4 < vtype > *v;
	vtype4_wzwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wzww_ref
{
	vtype4 < vtype > *v;
	vtype4_wzww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwxx_ref
{
	vtype4 < vtype > *v;
	vtype4_wwxx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwxy_ref
{
	vtype4 < vtype > *v;
	vtype4_wwxy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwxz_ref
{
	vtype4 < vtype > *v;
	vtype4_wwxz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwxw_ref
{
	vtype4 < vtype > *v;
	vtype4_wwxw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwyx_ref
{
	vtype4 < vtype > *v;
	vtype4_wwyx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwyy_ref
{
	vtype4 < vtype > *v;
	vtype4_wwyy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwyz_ref
{
	vtype4 < vtype > *v;
	vtype4_wwyz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwyw_ref
{
	vtype4 < vtype > *v;
	vtype4_wwyw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwzx_ref
{
	vtype4 < vtype > *v;
	vtype4_wwzx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwzy_ref
{
	vtype4 < vtype > *v;
	vtype4_wwzy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwzz_ref
{
	vtype4 < vtype > *v;
	vtype4_wwzz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwzw_ref
{
	vtype4 < vtype > *v;
	vtype4_wwzw_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwwx_ref
{
	vtype4 < vtype > *v;
	vtype4_wwwx_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwwy_ref
{
	vtype4 < vtype > *v;
	vtype4_wwwy_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwwz_ref
{
	vtype4 < vtype > *v;
	vtype4_wwwz_ref(vtype4 < vtype > *p):v(p)
	{
	};
};
template < typename vtype > struct vtype4_wwww_ref
{
	vtype4 < vtype > *v;
	vtype4_wwww_ref(vtype4 < vtype > *p):v(p)
	{
	};
};

template < typename vtype > class vtype2
{
  public:
	vtype x, y;
	vtype2()
	{
	};
  vtype2(vtype p, vtype q):x(p), y(q)
	{
	};
	vtype2(const vtype2 & p):x(p.x), y(p.y)
	{
	};
	inline vtype2(const vtype2_xx_ref < vtype > &v);
	inline vtype2(const vtype2_xy_ref < vtype > &v);
	inline vtype2(const vtype2_yx_ref < vtype > &v);
	inline vtype2(const vtype2_yy_ref < vtype > &v);
	inline vtype2(const vtype3_xx_ref < vtype > &v);
	inline vtype2(const vtype3_xy_ref < vtype > &v);
	inline vtype2(const vtype3_xz_ref < vtype > &v);
	inline vtype2(const vtype3_yx_ref < vtype > &v);
	inline vtype2(const vtype3_yy_ref < vtype > &v);
	inline vtype2(const vtype3_yz_ref < vtype > &v);
	inline vtype2(const vtype3_zx_ref < vtype > &v);
	inline vtype2(const vtype3_zy_ref < vtype > &v);
	inline vtype2(const vtype3_zz_ref < vtype > &v);
	inline vtype2(const vtype4_xx_ref < vtype > &v);
	inline vtype2(const vtype4_xy_ref < vtype > &v);
	inline vtype2(const vtype4_xz_ref < vtype > &v);
	inline vtype2(const vtype4_xw_ref < vtype > &v);
	inline vtype2(const vtype4_yx_ref < vtype > &v);
	inline vtype2(const vtype4_yy_ref < vtype > &v);
	inline vtype2(const vtype4_yz_ref < vtype > &v);
	inline vtype2(const vtype4_yw_ref < vtype > &v);
	inline vtype2(const vtype4_zx_ref < vtype > &v);
	inline vtype2(const vtype4_zy_ref < vtype > &v);
	inline vtype2(const vtype4_zz_ref < vtype > &v);
	inline vtype2(const vtype4_zw_ref < vtype > &v);
	inline vtype2(const vtype4_wx_ref < vtype > &v);
	inline vtype2(const vtype4_wy_ref < vtype > &v);
	inline vtype2(const vtype4_wz_ref < vtype > &v);
	inline vtype2(const vtype4_ww_ref < vtype > &v);
	vtype2_xx_ref < vtype > xx()
	{
		return vtype2_xx_ref < vtype > (this);
	}
	vtype2_xy_ref < vtype > xy()
	{
		return vtype2_xy_ref < vtype > (this);
	}
	vtype2_yx_ref < vtype > yx()
	{
		return vtype2_yx_ref < vtype > (this);
	}
	vtype2_yy_ref < vtype > yy()
	{
		return vtype2_yy_ref < vtype > (this);
	}
	vtype2_xxx_ref < vtype > xxx()
	{
		return vtype2_xxx_ref < vtype > (this);
	}
	vtype2_xxy_ref < vtype > xxy()
	{
		return vtype2_xxy_ref < vtype > (this);
	}
	vtype2_xyx_ref < vtype > xyx()
	{
		return vtype2_xyx_ref < vtype > (this);
	}
	vtype2_xyy_ref < vtype > xyy()
	{
		return vtype2_xyy_ref < vtype > (this);
	}
	vtype2_yxx_ref < vtype > yxx()
	{
		return vtype2_yxx_ref < vtype > (this);
	}
	vtype2_yxy_ref < vtype > yxy()
	{
		return vtype2_yxy_ref < vtype > (this);
	}
	vtype2_yyx_ref < vtype > yyx()
	{
		return vtype2_yyx_ref < vtype > (this);
	}
	vtype2_yyy_ref < vtype > yyy()
	{
		return vtype2_yyy_ref < vtype > (this);
	}
	vtype2_xxxx_ref < vtype > xxxx()
	{
		return vtype2_xxxx_ref < vtype > (this);
	}
	vtype2_xxxy_ref < vtype > xxxy()
	{
		return vtype2_xxxy_ref < vtype > (this);
	}
	vtype2_xxyx_ref < vtype > xxyx()
	{
		return vtype2_xxyx_ref < vtype > (this);
	}
	vtype2_xxyy_ref < vtype > xxyy()
	{
		return vtype2_xxyy_ref < vtype > (this);
	}
	vtype2_xyxx_ref < vtype > xyxx()
	{
		return vtype2_xyxx_ref < vtype > (this);
	}
	vtype2_xyxy_ref < vtype > xyxy()
	{
		return vtype2_xyxy_ref < vtype > (this);
	}
	vtype2_xyyx_ref < vtype > xyyx()
	{
		return vtype2_xyyx_ref < vtype > (this);
	}
	vtype2_xyyy_ref < vtype > xyyy()
	{
		return vtype2_xyyy_ref < vtype > (this);
	}
	vtype2_yxxx_ref < vtype > yxxx()
	{
		return vtype2_yxxx_ref < vtype > (this);
	}
	vtype2_yxxy_ref < vtype > yxxy()
	{
		return vtype2_yxxy_ref < vtype > (this);
	}
	vtype2_yxyx_ref < vtype > yxyx()
	{
		return vtype2_yxyx_ref < vtype > (this);
	}
	vtype2_yxyy_ref < vtype > yxyy()
	{
		return vtype2_yxyy_ref < vtype > (this);
	}
	vtype2_yyxx_ref < vtype > yyxx()
	{
		return vtype2_yyxx_ref < vtype > (this);
	}
	vtype2_yyxy_ref < vtype > yyxy()
	{
		return vtype2_yyxy_ref < vtype > (this);
	}
	vtype2_yyyx_ref < vtype > yyyx()
	{
		return vtype2_yyyx_ref < vtype > (this);
	}
	vtype2_yyyy_ref < vtype > yyyy()
	{
		return vtype2_yyyy_ref < vtype > (this);
	}
};

template < typename vtype > class vtype3
{
  public:
	vtype x, y, z;
	vtype3()
	{
	};
  vtype3(vtype p, vtype q, vtype r):x(p), y(q), z(r)
	{
	};
  vtype3(const vtype3 & p):x(p.x), y(p.y), z(p.z)
	{
	};
	vtype3(vtype p, const vtype2 < vtype > &q):x(p), y(q.x), z(q.y)
	{
	};
	vtype3(const vtype2 < vtype > &p, vtype q):x(p.x), y(p.y), z(q)
	{
	};
	inline vtype3(const vtype2_xxx_ref < vtype > &v);
	inline vtype3(const vtype2_xxy_ref < vtype > &v);
	inline vtype3(const vtype2_xyx_ref < vtype > &v);
	inline vtype3(const vtype2_xyy_ref < vtype > &v);
	inline vtype3(const vtype2_yxx_ref < vtype > &v);
	inline vtype3(const vtype2_yxy_ref < vtype > &v);
	inline vtype3(const vtype2_yyx_ref < vtype > &v);
	inline vtype3(const vtype2_yyy_ref < vtype > &v);
	inline vtype3(const vtype3_xxx_ref < vtype > &v);
	inline vtype3(const vtype3_xxy_ref < vtype > &v);
	inline vtype3(const vtype3_xxz_ref < vtype > &v);
	inline vtype3(const vtype3_xyx_ref < vtype > &v);
	inline vtype3(const vtype3_xyy_ref < vtype > &v);
	inline vtype3(const vtype3_xyz_ref < vtype > &v);
	inline vtype3(const vtype3_xzx_ref < vtype > &v);
	inline vtype3(const vtype3_xzy_ref < vtype > &v);
	inline vtype3(const vtype3_xzz_ref < vtype > &v);
	inline vtype3(const vtype3_yxx_ref < vtype > &v);
	inline vtype3(const vtype3_yxy_ref < vtype > &v);
	inline vtype3(const vtype3_yxz_ref < vtype > &v);
	inline vtype3(const vtype3_yyx_ref < vtype > &v);
	inline vtype3(const vtype3_yyy_ref < vtype > &v);
	inline vtype3(const vtype3_yyz_ref < vtype > &v);
	inline vtype3(const vtype3_yzx_ref < vtype > &v);
	inline vtype3(const vtype3_yzy_ref < vtype > &v);
	inline vtype3(const vtype3_yzz_ref < vtype > &v);
	inline vtype3(const vtype3_zxx_ref < vtype > &v);
	inline vtype3(const vtype3_zxy_ref < vtype > &v);
	inline vtype3(const vtype3_zxz_ref < vtype > &v);
	inline vtype3(const vtype3_zyx_ref < vtype > &v);
	inline vtype3(const vtype3_zyy_ref < vtype > &v);
	inline vtype3(const vtype3_zyz_ref < vtype > &v);
	inline vtype3(const vtype3_zzx_ref < vtype > &v);
	inline vtype3(const vtype3_zzy_ref < vtype > &v);
	inline vtype3(const vtype3_zzz_ref < vtype > &v);
	inline vtype3(const vtype4_xxx_ref < vtype > &v);
	inline vtype3(const vtype4_xxy_ref < vtype > &v);
	inline vtype3(const vtype4_xxz_ref < vtype > &v);
	inline vtype3(const vtype4_xxw_ref < vtype > &v);
	inline vtype3(const vtype4_xyx_ref < vtype > &v);
	inline vtype3(const vtype4_xyy_ref < vtype > &v);
	inline vtype3(const vtype4_xyz_ref < vtype > &v);
	inline vtype3(const vtype4_xyw_ref < vtype > &v);
	inline vtype3(const vtype4_xzx_ref < vtype > &v);
	inline vtype3(const vtype4_xzy_ref < vtype > &v);
	inline vtype3(const vtype4_xzz_ref < vtype > &v);
	inline vtype3(const vtype4_xzw_ref < vtype > &v);
	inline vtype3(const vtype4_xwx_ref < vtype > &v);
	inline vtype3(const vtype4_xwy_ref < vtype > &v);
	inline vtype3(const vtype4_xwz_ref < vtype > &v);
	inline vtype3(const vtype4_xww_ref < vtype > &v);
	inline vtype3(const vtype4_yxx_ref < vtype > &v);
	inline vtype3(const vtype4_yxy_ref < vtype > &v);
	inline vtype3(const vtype4_yxz_ref < vtype > &v);
	inline vtype3(const vtype4_yxw_ref < vtype > &v);
	inline vtype3(const vtype4_yyx_ref < vtype > &v);
	inline vtype3(const vtype4_yyy_ref < vtype > &v);
	inline vtype3(const vtype4_yyz_ref < vtype > &v);
	inline vtype3(const vtype4_yyw_ref < vtype > &v);
	inline vtype3(const vtype4_yzx_ref < vtype > &v);
	inline vtype3(const vtype4_yzy_ref < vtype > &v);
	inline vtype3(const vtype4_yzz_ref < vtype > &v);
	inline vtype3(const vtype4_yzw_ref < vtype > &v);
	inline vtype3(const vtype4_ywx_ref < vtype > &v);
	inline vtype3(const vtype4_ywy_ref < vtype > &v);
	inline vtype3(const vtype4_ywz_ref < vtype > &v);
	inline vtype3(const vtype4_yww_ref < vtype > &v);
	inline vtype3(const vtype4_zxx_ref < vtype > &v);
	inline vtype3(const vtype4_zxy_ref < vtype > &v);
	inline vtype3(const vtype4_zxz_ref < vtype > &v);
	inline vtype3(const vtype4_zxw_ref < vtype > &v);
	inline vtype3(const vtype4_zyx_ref < vtype > &v);
	inline vtype3(const vtype4_zyy_ref < vtype > &v);
	inline vtype3(const vtype4_zyz_ref < vtype > &v);
	inline vtype3(const vtype4_zyw_ref < vtype > &v);
	inline vtype3(const vtype4_zzx_ref < vtype > &v);
	inline vtype3(const vtype4_zzy_ref < vtype > &v);
	inline vtype3(const vtype4_zzz_ref < vtype > &v);
	inline vtype3(const vtype4_zzw_ref < vtype > &v);
	inline vtype3(const vtype4_zwx_ref < vtype > &v);
	inline vtype3(const vtype4_zwy_ref < vtype > &v);
	inline vtype3(const vtype4_zwz_ref < vtype > &v);
	inline vtype3(const vtype4_zww_ref < vtype > &v);
	inline vtype3(const vtype4_wxx_ref < vtype > &v);
	inline vtype3(const vtype4_wxy_ref < vtype > &v);
	inline vtype3(const vtype4_wxz_ref < vtype > &v);
	inline vtype3(const vtype4_wxw_ref < vtype > &v);
	inline vtype3(const vtype4_wyx_ref < vtype > &v);
	inline vtype3(const vtype4_wyy_ref < vtype > &v);
	inline vtype3(const vtype4_wyz_ref < vtype > &v);
	inline vtype3(const vtype4_wyw_ref < vtype > &v);
	inline vtype3(const vtype4_wzx_ref < vtype > &v);
	inline vtype3(const vtype4_wzy_ref < vtype > &v);
	inline vtype3(const vtype4_wzz_ref < vtype > &v);
	inline vtype3(const vtype4_wzw_ref < vtype > &v);
	inline vtype3(const vtype4_wwx_ref < vtype > &v);
	inline vtype3(const vtype4_wwy_ref < vtype > &v);
	inline vtype3(const vtype4_wwz_ref < vtype > &v);
	inline vtype3(const vtype4_www_ref < vtype > &v);
	vtype3_xx_ref < vtype > xx()
	{
		return vtype3_xx_ref < vtype > (this);
	}
	vtype3_xy_ref < vtype > xy()
	{
		return vtype3_xy_ref < vtype > (this);
	}
	vtype3_xz_ref < vtype > xz()
	{
		return vtype3_xz_ref < vtype > (this);
	}
	vtype3_yx_ref < vtype > yx()
	{
		return vtype3_yx_ref < vtype > (this);
	}
	vtype3_yy_ref < vtype > yy()
	{
		return vtype3_yy_ref < vtype > (this);
	}
	vtype3_yz_ref < vtype > yz()
	{
		return vtype3_yz_ref < vtype > (this);
	}
	vtype3_zx_ref < vtype > zx()
	{
		return vtype3_zx_ref < vtype > (this);
	}
	vtype3_zy_ref < vtype > zy()
	{
		return vtype3_zy_ref < vtype > (this);
	}
	vtype3_zz_ref < vtype > zz()
	{
		return vtype3_zz_ref < vtype > (this);
	}
	vtype3_xxx_ref < vtype > xxx()
	{
		return vtype3_xxx_ref < vtype > (this);
	}
	vtype3_xxy_ref < vtype > xxy()
	{
		return vtype3_xxy_ref < vtype > (this);
	}
	vtype3_xxz_ref < vtype > xxz()
	{
		return vtype3_xxz_ref < vtype > (this);
	}
	vtype3_xyx_ref < vtype > xyx()
	{
		return vtype3_xyx_ref < vtype > (this);
	}
	vtype3_xyy_ref < vtype > xyy()
	{
		return vtype3_xyy_ref < vtype > (this);
	}
	vtype3_xyz_ref < vtype > xyz()
	{
		return vtype3_xyz_ref < vtype > (this);
	}
	vtype3_xzx_ref < vtype > xzx()
	{
		return vtype3_xzx_ref < vtype > (this);
	}
	vtype3_xzy_ref < vtype > xzy()
	{
		return vtype3_xzy_ref < vtype > (this);
	}
	vtype3_xzz_ref < vtype > xzz()
	{
		return vtype3_xzz_ref < vtype > (this);
	}
	vtype3_yxx_ref < vtype > yxx()
	{
		return vtype3_yxx_ref < vtype > (this);
	}
	vtype3_yxy_ref < vtype > yxy()
	{
		return vtype3_yxy_ref < vtype > (this);
	}
	vtype3_yxz_ref < vtype > yxz()
	{
		return vtype3_yxz_ref < vtype > (this);
	}
	vtype3_yyx_ref < vtype > yyx()
	{
		return vtype3_yyx_ref < vtype > (this);
	}
	vtype3_yyy_ref < vtype > yyy()
	{
		return vtype3_yyy_ref < vtype > (this);
	}
	vtype3_yyz_ref < vtype > yyz()
	{
		return vtype3_yyz_ref < vtype > (this);
	}
	vtype3_yzx_ref < vtype > yzx()
	{
		return vtype3_yzx_ref < vtype > (this);
	}
	vtype3_yzy_ref < vtype > yzy()
	{
		return vtype3_yzy_ref < vtype > (this);
	}
	vtype3_yzz_ref < vtype > yzz()
	{
		return vtype3_yzz_ref < vtype > (this);
	}
	vtype3_zxx_ref < vtype > zxx()
	{
		return vtype3_zxx_ref < vtype > (this);
	}
	vtype3_zxy_ref < vtype > zxy()
	{
		return vtype3_zxy_ref < vtype > (this);
	}
	vtype3_zxz_ref < vtype > zxz()
	{
		return vtype3_zxz_ref < vtype > (this);
	}
	vtype3_zyx_ref < vtype > zyx()
	{
		return vtype3_zyx_ref < vtype > (this);
	}
	vtype3_zyy_ref < vtype > zyy()
	{
		return vtype3_zyy_ref < vtype > (this);
	}
	vtype3_zyz_ref < vtype > zyz()
	{
		return vtype3_zyz_ref < vtype > (this);
	}
	vtype3_zzx_ref < vtype > zzx()
	{
		return vtype3_zzx_ref < vtype > (this);
	}
	vtype3_zzy_ref < vtype > zzy()
	{
		return vtype3_zzy_ref < vtype > (this);
	}
	vtype3_zzz_ref < vtype > zzz()
	{
		return vtype3_zzz_ref < vtype > (this);
	}
	vtype3_xxxx_ref < vtype > xxxx()
	{
		return vtype3_xxxx_ref < vtype > (this);
	}
	vtype3_xxxy_ref < vtype > xxxy()
	{
		return vtype3_xxxy_ref < vtype > (this);
	}
	vtype3_xxxz_ref < vtype > xxxz()
	{
		return vtype3_xxxz_ref < vtype > (this);
	}
	vtype3_xxyx_ref < vtype > xxyx()
	{
		return vtype3_xxyx_ref < vtype > (this);
	}
	vtype3_xxyy_ref < vtype > xxyy()
	{
		return vtype3_xxyy_ref < vtype > (this);
	}
	vtype3_xxyz_ref < vtype > xxyz()
	{
		return vtype3_xxyz_ref < vtype > (this);
	}
	vtype3_xxzx_ref < vtype > xxzx()
	{
		return vtype3_xxzx_ref < vtype > (this);
	}
	vtype3_xxzy_ref < vtype > xxzy()
	{
		return vtype3_xxzy_ref < vtype > (this);
	}
	vtype3_xxzz_ref < vtype > xxzz()
	{
		return vtype3_xxzz_ref < vtype > (this);
	}
	vtype3_xyxx_ref < vtype > xyxx()
	{
		return vtype3_xyxx_ref < vtype > (this);
	}
	vtype3_xyxy_ref < vtype > xyxy()
	{
		return vtype3_xyxy_ref < vtype > (this);
	}
	vtype3_xyxz_ref < vtype > xyxz()
	{
		return vtype3_xyxz_ref < vtype > (this);
	}
	vtype3_xyyx_ref < vtype > xyyx()
	{
		return vtype3_xyyx_ref < vtype > (this);
	}
	vtype3_xyyy_ref < vtype > xyyy()
	{
		return vtype3_xyyy_ref < vtype > (this);
	}
	vtype3_xyyz_ref < vtype > xyyz()
	{
		return vtype3_xyyz_ref < vtype > (this);
	}
	vtype3_xyzx_ref < vtype > xyzx()
	{
		return vtype3_xyzx_ref < vtype > (this);
	}
	vtype3_xyzy_ref < vtype > xyzy()
	{
		return vtype3_xyzy_ref < vtype > (this);
	}
	vtype3_xyzz_ref < vtype > xyzz()
	{
		return vtype3_xyzz_ref < vtype > (this);
	}
	vtype3_xzxx_ref < vtype > xzxx()
	{
		return vtype3_xzxx_ref < vtype > (this);
	}
	vtype3_xzxy_ref < vtype > xzxy()
	{
		return vtype3_xzxy_ref < vtype > (this);
	}
	vtype3_xzxz_ref < vtype > xzxz()
	{
		return vtype3_xzxz_ref < vtype > (this);
	}
	vtype3_xzyx_ref < vtype > xzyx()
	{
		return vtype3_xzyx_ref < vtype > (this);
	}
	vtype3_xzyy_ref < vtype > xzyy()
	{
		return vtype3_xzyy_ref < vtype > (this);
	}
	vtype3_xzyz_ref < vtype > xzyz()
	{
		return vtype3_xzyz_ref < vtype > (this);
	}
	vtype3_xzzx_ref < vtype > xzzx()
	{
		return vtype3_xzzx_ref < vtype > (this);
	}
	vtype3_xzzy_ref < vtype > xzzy()
	{
		return vtype3_xzzy_ref < vtype > (this);
	}
	vtype3_xzzz_ref < vtype > xzzz()
	{
		return vtype3_xzzz_ref < vtype > (this);
	}
	vtype3_yxxx_ref < vtype > yxxx()
	{
		return vtype3_yxxx_ref < vtype > (this);
	}
	vtype3_yxxy_ref < vtype > yxxy()
	{
		return vtype3_yxxy_ref < vtype > (this);
	}
	vtype3_yxxz_ref < vtype > yxxz()
	{
		return vtype3_yxxz_ref < vtype > (this);
	}
	vtype3_yxyx_ref < vtype > yxyx()
	{
		return vtype3_yxyx_ref < vtype > (this);
	}
	vtype3_yxyy_ref < vtype > yxyy()
	{
		return vtype3_yxyy_ref < vtype > (this);
	}
	vtype3_yxyz_ref < vtype > yxyz()
	{
		return vtype3_yxyz_ref < vtype > (this);
	}
	vtype3_yxzx_ref < vtype > yxzx()
	{
		return vtype3_yxzx_ref < vtype > (this);
	}
	vtype3_yxzy_ref < vtype > yxzy()
	{
		return vtype3_yxzy_ref < vtype > (this);
	}
	vtype3_yxzz_ref < vtype > yxzz()
	{
		return vtype3_yxzz_ref < vtype > (this);
	}
	vtype3_yyxx_ref < vtype > yyxx()
	{
		return vtype3_yyxx_ref < vtype > (this);
	}
	vtype3_yyxy_ref < vtype > yyxy()
	{
		return vtype3_yyxy_ref < vtype > (this);
	}
	vtype3_yyxz_ref < vtype > yyxz()
	{
		return vtype3_yyxz_ref < vtype > (this);
	}
	vtype3_yyyx_ref < vtype > yyyx()
	{
		return vtype3_yyyx_ref < vtype > (this);
	}
	vtype3_yyyy_ref < vtype > yyyy()
	{
		return vtype3_yyyy_ref < vtype > (this);
	}
	vtype3_yyyz_ref < vtype > yyyz()
	{
		return vtype3_yyyz_ref < vtype > (this);
	}
	vtype3_yyzx_ref < vtype > yyzx()
	{
		return vtype3_yyzx_ref < vtype > (this);
	}
	vtype3_yyzy_ref < vtype > yyzy()
	{
		return vtype3_yyzy_ref < vtype > (this);
	}
	vtype3_yyzz_ref < vtype > yyzz()
	{
		return vtype3_yyzz_ref < vtype > (this);
	}
	vtype3_yzxx_ref < vtype > yzxx()
	{
		return vtype3_yzxx_ref < vtype > (this);
	}
	vtype3_yzxy_ref < vtype > yzxy()
	{
		return vtype3_yzxy_ref < vtype > (this);
	}
	vtype3_yzxz_ref < vtype > yzxz()
	{
		return vtype3_yzxz_ref < vtype > (this);
	}
	vtype3_yzyx_ref < vtype > yzyx()
	{
		return vtype3_yzyx_ref < vtype > (this);
	}
	vtype3_yzyy_ref < vtype > yzyy()
	{
		return vtype3_yzyy_ref < vtype > (this);
	}
	vtype3_yzyz_ref < vtype > yzyz()
	{
		return vtype3_yzyz_ref < vtype > (this);
	}
	vtype3_yzzx_ref < vtype > yzzx()
	{
		return vtype3_yzzx_ref < vtype > (this);
	}
	vtype3_yzzy_ref < vtype > yzzy()
	{
		return vtype3_yzzy_ref < vtype > (this);
	}
	vtype3_yzzz_ref < vtype > yzzz()
	{
		return vtype3_yzzz_ref < vtype > (this);
	}
	vtype3_zxxx_ref < vtype > zxxx()
	{
		return vtype3_zxxx_ref < vtype > (this);
	}
	vtype3_zxxy_ref < vtype > zxxy()
	{
		return vtype3_zxxy_ref < vtype > (this);
	}
	vtype3_zxxz_ref < vtype > zxxz()
	{
		return vtype3_zxxz_ref < vtype > (this);
	}
	vtype3_zxyx_ref < vtype > zxyx()
	{
		return vtype3_zxyx_ref < vtype > (this);
	}
	vtype3_zxyy_ref < vtype > zxyy()
	{
		return vtype3_zxyy_ref < vtype > (this);
	}
	vtype3_zxyz_ref < vtype > zxyz()
	{
		return vtype3_zxyz_ref < vtype > (this);
	}
	vtype3_zxzx_ref < vtype > zxzx()
	{
		return vtype3_zxzx_ref < vtype > (this);
	}
	vtype3_zxzy_ref < vtype > zxzy()
	{
		return vtype3_zxzy_ref < vtype > (this);
	}
	vtype3_zxzz_ref < vtype > zxzz()
	{
		return vtype3_zxzz_ref < vtype > (this);
	}
	vtype3_zyxx_ref < vtype > zyxx()
	{
		return vtype3_zyxx_ref < vtype > (this);
	}
	vtype3_zyxy_ref < vtype > zyxy()
	{
		return vtype3_zyxy_ref < vtype > (this);
	}
	vtype3_zyxz_ref < vtype > zyxz()
	{
		return vtype3_zyxz_ref < vtype > (this);
	}
	vtype3_zyyx_ref < vtype > zyyx()
	{
		return vtype3_zyyx_ref < vtype > (this);
	}
	vtype3_zyyy_ref < vtype > zyyy()
	{
		return vtype3_zyyy_ref < vtype > (this);
	}
	vtype3_zyyz_ref < vtype > zyyz()
	{
		return vtype3_zyyz_ref < vtype > (this);
	}
	vtype3_zyzx_ref < vtype > zyzx()
	{
		return vtype3_zyzx_ref < vtype > (this);
	}
	vtype3_zyzy_ref < vtype > zyzy()
	{
		return vtype3_zyzy_ref < vtype > (this);
	}
	vtype3_zyzz_ref < vtype > zyzz()
	{
		return vtype3_zyzz_ref < vtype > (this);
	}
	vtype3_zzxx_ref < vtype > zzxx()
	{
		return vtype3_zzxx_ref < vtype > (this);
	}
	vtype3_zzxy_ref < vtype > zzxy()
	{
		return vtype3_zzxy_ref < vtype > (this);
	}
	vtype3_zzxz_ref < vtype > zzxz()
	{
		return vtype3_zzxz_ref < vtype > (this);
	}
	vtype3_zzyx_ref < vtype > zzyx()
	{
		return vtype3_zzyx_ref < vtype > (this);
	}
	vtype3_zzyy_ref < vtype > zzyy()
	{
		return vtype3_zzyy_ref < vtype > (this);
	}
	vtype3_zzyz_ref < vtype > zzyz()
	{
		return vtype3_zzyz_ref < vtype > (this);
	}
	vtype3_zzzx_ref < vtype > zzzx()
	{
		return vtype3_zzzx_ref < vtype > (this);
	}
	vtype3_zzzy_ref < vtype > zzzy()
	{
		return vtype3_zzzy_ref < vtype > (this);
	}
	vtype3_zzzz_ref < vtype > zzzz()
	{
		return vtype3_zzzz_ref < vtype > (this);
	}
};

template < typename vtype > class vtype4
{
  public:
	vtype x, y, z, w;
	vtype4()
	{
	};
  vtype4(vtype p, vtype q, vtype r, vtype s):x(p), y(q), z(r), w(s)
	{
	};
  vtype4(const vtype2 < vtype > &p, const vtype2 < vtype > &q):x(p.x), y(p.y), z(q.x), w(q.y)
	{
	};
	vtype4(const vtype2 < vtype > &p, vtype q, vtype r):x(p.x), y(p.y), z(q), w(r)
	{
	};
	vtype4(vtype p, const vtype2 < vtype > &q, vtype r):x(p), y(q.x), z(q.y), w(r)
	{
	};
	vtype4(vtype p, vtype q, const vtype2 < vtype > &r):x(p), y(q), z(r.x), w(r.y)
	{
	};
	vtype4(const vtype3 < vtype > &p, vtype q):x(p.x), y(p.y), z(p.z), w(q)
	{
	};
	vtype4(vtype p, const vtype3 < vtype > &q):x(p), y(q.x), z(q.y), w(q.z)
	{
	};
	vtype4(const vtype4 & p):x(p.x), y(p.y), z(p.z), w(p.w)
	{
	};
	inline vtype4(const vtype2_xxxx_ref < vtype > &v);
	inline vtype4(const vtype2_xxxy_ref < vtype > &v);
	inline vtype4(const vtype2_xxyx_ref < vtype > &v);
	inline vtype4(const vtype2_xxyy_ref < vtype > &v);
	inline vtype4(const vtype2_xyxx_ref < vtype > &v);
	inline vtype4(const vtype2_xyxy_ref < vtype > &v);
	inline vtype4(const vtype2_xyyx_ref < vtype > &v);
	inline vtype4(const vtype2_xyyy_ref < vtype > &v);
	inline vtype4(const vtype2_yxxx_ref < vtype > &v);
	inline vtype4(const vtype2_yxxy_ref < vtype > &v);
	inline vtype4(const vtype2_yxyx_ref < vtype > &v);
	inline vtype4(const vtype2_yxyy_ref < vtype > &v);
	inline vtype4(const vtype2_yyxx_ref < vtype > &v);
	inline vtype4(const vtype2_yyxy_ref < vtype > &v);
	inline vtype4(const vtype2_yyyx_ref < vtype > &v);
	inline vtype4(const vtype2_yyyy_ref < vtype > &v);
	inline vtype4(const vtype3_xxxx_ref < vtype > &v);
	inline vtype4(const vtype3_xxxy_ref < vtype > &v);
	inline vtype4(const vtype3_xxxz_ref < vtype > &v);
	inline vtype4(const vtype3_xxyx_ref < vtype > &v);
	inline vtype4(const vtype3_xxyy_ref < vtype > &v);
	inline vtype4(const vtype3_xxyz_ref < vtype > &v);
	inline vtype4(const vtype3_xxzx_ref < vtype > &v);
	inline vtype4(const vtype3_xxzy_ref < vtype > &v);
	inline vtype4(const vtype3_xxzz_ref < vtype > &v);
	inline vtype4(const vtype3_xyxx_ref < vtype > &v);
	inline vtype4(const vtype3_xyxy_ref < vtype > &v);
	inline vtype4(const vtype3_xyxz_ref < vtype > &v);
	inline vtype4(const vtype3_xyyx_ref < vtype > &v);
	inline vtype4(const vtype3_xyyy_ref < vtype > &v);
	inline vtype4(const vtype3_xyyz_ref < vtype > &v);
	inline vtype4(const vtype3_xyzx_ref < vtype > &v);
	inline vtype4(const vtype3_xyzy_ref < vtype > &v);
	inline vtype4(const vtype3_xyzz_ref < vtype > &v);
	inline vtype4(const vtype3_xzxx_ref < vtype > &v);
	inline vtype4(const vtype3_xzxy_ref < vtype > &v);
	inline vtype4(const vtype3_xzxz_ref < vtype > &v);
	inline vtype4(const vtype3_xzyx_ref < vtype > &v);
	inline vtype4(const vtype3_xzyy_ref < vtype > &v);
	inline vtype4(const vtype3_xzyz_ref < vtype > &v);
	inline vtype4(const vtype3_xzzx_ref < vtype > &v);
	inline vtype4(const vtype3_xzzy_ref < vtype > &v);
	inline vtype4(const vtype3_xzzz_ref < vtype > &v);
	inline vtype4(const vtype3_yxxx_ref < vtype > &v);
	inline vtype4(const vtype3_yxxy_ref < vtype > &v);
	inline vtype4(const vtype3_yxxz_ref < vtype > &v);
	inline vtype4(const vtype3_yxyx_ref < vtype > &v);
	inline vtype4(const vtype3_yxyy_ref < vtype > &v);
	inline vtype4(const vtype3_yxyz_ref < vtype > &v);
	inline vtype4(const vtype3_yxzx_ref < vtype > &v);
	inline vtype4(const vtype3_yxzy_ref < vtype > &v);
	inline vtype4(const vtype3_yxzz_ref < vtype > &v);
	inline vtype4(const vtype3_yyxx_ref < vtype > &v);
	inline vtype4(const vtype3_yyxy_ref < vtype > &v);
	inline vtype4(const vtype3_yyxz_ref < vtype > &v);
	inline vtype4(const vtype3_yyyx_ref < vtype > &v);
	inline vtype4(const vtype3_yyyy_ref < vtype > &v);
	inline vtype4(const vtype3_yyyz_ref < vtype > &v);
	inline vtype4(const vtype3_yyzx_ref < vtype > &v);
	inline vtype4(const vtype3_yyzy_ref < vtype > &v);
	inline vtype4(const vtype3_yyzz_ref < vtype > &v);
	inline vtype4(const vtype3_yzxx_ref < vtype > &v);
	inline vtype4(const vtype3_yzxy_ref < vtype > &v);
	inline vtype4(const vtype3_yzxz_ref < vtype > &v);
	inline vtype4(const vtype3_yzyx_ref < vtype > &v);
	inline vtype4(const vtype3_yzyy_ref < vtype > &v);
	inline vtype4(const vtype3_yzyz_ref < vtype > &v);
	inline vtype4(const vtype3_yzzx_ref < vtype > &v);
	inline vtype4(const vtype3_yzzy_ref < vtype > &v);
	inline vtype4(const vtype3_yzzz_ref < vtype > &v);
	inline vtype4(const vtype3_zxxx_ref < vtype > &v);
	inline vtype4(const vtype3_zxxy_ref < vtype > &v);
	inline vtype4(const vtype3_zxxz_ref < vtype > &v);
	inline vtype4(const vtype3_zxyx_ref < vtype > &v);
	inline vtype4(const vtype3_zxyy_ref < vtype > &v);
	inline vtype4(const vtype3_zxyz_ref < vtype > &v);
	inline vtype4(const vtype3_zxzx_ref < vtype > &v);
	inline vtype4(const vtype3_zxzy_ref < vtype > &v);
	inline vtype4(const vtype3_zxzz_ref < vtype > &v);
	inline vtype4(const vtype3_zyxx_ref < vtype > &v);
	inline vtype4(const vtype3_zyxy_ref < vtype > &v);
	inline vtype4(const vtype3_zyxz_ref < vtype > &v);
	inline vtype4(const vtype3_zyyx_ref < vtype > &v);
	inline vtype4(const vtype3_zyyy_ref < vtype > &v);
	inline vtype4(const vtype3_zyyz_ref < vtype > &v);
	inline vtype4(const vtype3_zyzx_ref < vtype > &v);
	inline vtype4(const vtype3_zyzy_ref < vtype > &v);
	inline vtype4(const vtype3_zyzz_ref < vtype > &v);
	inline vtype4(const vtype3_zzxx_ref < vtype > &v);
	inline vtype4(const vtype3_zzxy_ref < vtype > &v);
	inline vtype4(const vtype3_zzxz_ref < vtype > &v);
	inline vtype4(const vtype3_zzyx_ref < vtype > &v);
	inline vtype4(const vtype3_zzyy_ref < vtype > &v);
	inline vtype4(const vtype3_zzyz_ref < vtype > &v);
	inline vtype4(const vtype3_zzzx_ref < vtype > &v);
	inline vtype4(const vtype3_zzzy_ref < vtype > &v);
	inline vtype4(const vtype3_zzzz_ref < vtype > &v);
	inline vtype4(const vtype4_xxxx_ref < vtype > &v);
	inline vtype4(const vtype4_xxxy_ref < vtype > &v);
	inline vtype4(const vtype4_xxxz_ref < vtype > &v);
	inline vtype4(const vtype4_xxxw_ref < vtype > &v);
	inline vtype4(const vtype4_xxyx_ref < vtype > &v);
	inline vtype4(const vtype4_xxyy_ref < vtype > &v);
	inline vtype4(const vtype4_xxyz_ref < vtype > &v);
	inline vtype4(const vtype4_xxyw_ref < vtype > &v);
	inline vtype4(const vtype4_xxzx_ref < vtype > &v);
	inline vtype4(const vtype4_xxzy_ref < vtype > &v);
	inline vtype4(const vtype4_xxzz_ref < vtype > &v);
	inline vtype4(const vtype4_xxzw_ref < vtype > &v);
	inline vtype4(const vtype4_xxwx_ref < vtype > &v);
	inline vtype4(const vtype4_xxwy_ref < vtype > &v);
	inline vtype4(const vtype4_xxwz_ref < vtype > &v);
	inline vtype4(const vtype4_xxww_ref < vtype > &v);
	inline vtype4(const vtype4_xyxx_ref < vtype > &v);
	inline vtype4(const vtype4_xyxy_ref < vtype > &v);
	inline vtype4(const vtype4_xyxz_ref < vtype > &v);
	inline vtype4(const vtype4_xyxw_ref < vtype > &v);
	inline vtype4(const vtype4_xyyx_ref < vtype > &v);
	inline vtype4(const vtype4_xyyy_ref < vtype > &v);
	inline vtype4(const vtype4_xyyz_ref < vtype > &v);
	inline vtype4(const vtype4_xyyw_ref < vtype > &v);
	inline vtype4(const vtype4_xyzx_ref < vtype > &v);
	inline vtype4(const vtype4_xyzy_ref < vtype > &v);
	inline vtype4(const vtype4_xyzz_ref < vtype > &v);
	inline vtype4(const vtype4_xyzw_ref < vtype > &v);
	inline vtype4(const vtype4_xywx_ref < vtype > &v);
	inline vtype4(const vtype4_xywy_ref < vtype > &v);
	inline vtype4(const vtype4_xywz_ref < vtype > &v);
	inline vtype4(const vtype4_xyww_ref < vtype > &v);
	inline vtype4(const vtype4_xzxx_ref < vtype > &v);
	inline vtype4(const vtype4_xzxy_ref < vtype > &v);
	inline vtype4(const vtype4_xzxz_ref < vtype > &v);
	inline vtype4(const vtype4_xzxw_ref < vtype > &v);
	inline vtype4(const vtype4_xzyx_ref < vtype > &v);
	inline vtype4(const vtype4_xzyy_ref < vtype > &v);
	inline vtype4(const vtype4_xzyz_ref < vtype > &v);
	inline vtype4(const vtype4_xzyw_ref < vtype > &v);
	inline vtype4(const vtype4_xzzx_ref < vtype > &v);
	inline vtype4(const vtype4_xzzy_ref < vtype > &v);
	inline vtype4(const vtype4_xzzz_ref < vtype > &v);
	inline vtype4(const vtype4_xzzw_ref < vtype > &v);
	inline vtype4(const vtype4_xzwx_ref < vtype > &v);
	inline vtype4(const vtype4_xzwy_ref < vtype > &v);
	inline vtype4(const vtype4_xzwz_ref < vtype > &v);
	inline vtype4(const vtype4_xzww_ref < vtype > &v);
	inline vtype4(const vtype4_xwxx_ref < vtype > &v);
	inline vtype4(const vtype4_xwxy_ref < vtype > &v);
	inline vtype4(const vtype4_xwxz_ref < vtype > &v);
	inline vtype4(const vtype4_xwxw_ref < vtype > &v);
	inline vtype4(const vtype4_xwyx_ref < vtype > &v);
	inline vtype4(const vtype4_xwyy_ref < vtype > &v);
	inline vtype4(const vtype4_xwyz_ref < vtype > &v);
	inline vtype4(const vtype4_xwyw_ref < vtype > &v);
	inline vtype4(const vtype4_xwzx_ref < vtype > &v);
	inline vtype4(const vtype4_xwzy_ref < vtype > &v);
	inline vtype4(const vtype4_xwzz_ref < vtype > &v);
	inline vtype4(const vtype4_xwzw_ref < vtype > &v);
	inline vtype4(const vtype4_xwwx_ref < vtype > &v);
	inline vtype4(const vtype4_xwwy_ref < vtype > &v);
	inline vtype4(const vtype4_xwwz_ref < vtype > &v);
	inline vtype4(const vtype4_xwww_ref < vtype > &v);
	inline vtype4(const vtype4_yxxx_ref < vtype > &v);
	inline vtype4(const vtype4_yxxy_ref < vtype > &v);
	inline vtype4(const vtype4_yxxz_ref < vtype > &v);
	inline vtype4(const vtype4_yxxw_ref < vtype > &v);
	inline vtype4(const vtype4_yxyx_ref < vtype > &v);
	inline vtype4(const vtype4_yxyy_ref < vtype > &v);
	inline vtype4(const vtype4_yxyz_ref < vtype > &v);
	inline vtype4(const vtype4_yxyw_ref < vtype > &v);
	inline vtype4(const vtype4_yxzx_ref < vtype > &v);
	inline vtype4(const vtype4_yxzy_ref < vtype > &v);
	inline vtype4(const vtype4_yxzz_ref < vtype > &v);
	inline vtype4(const vtype4_yxzw_ref < vtype > &v);
	inline vtype4(const vtype4_yxwx_ref < vtype > &v);
	inline vtype4(const vtype4_yxwy_ref < vtype > &v);
	inline vtype4(const vtype4_yxwz_ref < vtype > &v);
	inline vtype4(const vtype4_yxww_ref < vtype > &v);
	inline vtype4(const vtype4_yyxx_ref < vtype > &v);
	inline vtype4(const vtype4_yyxy_ref < vtype > &v);
	inline vtype4(const vtype4_yyxz_ref < vtype > &v);
	inline vtype4(const vtype4_yyxw_ref < vtype > &v);
	inline vtype4(const vtype4_yyyx_ref < vtype > &v);
	inline vtype4(const vtype4_yyyy_ref < vtype > &v);
	inline vtype4(const vtype4_yyyz_ref < vtype > &v);
	inline vtype4(const vtype4_yyyw_ref < vtype > &v);
	inline vtype4(const vtype4_yyzx_ref < vtype > &v);
	inline vtype4(const vtype4_yyzy_ref < vtype > &v);
	inline vtype4(const vtype4_yyzz_ref < vtype > &v);
	inline vtype4(const vtype4_yyzw_ref < vtype > &v);
	inline vtype4(const vtype4_yywx_ref < vtype > &v);
	inline vtype4(const vtype4_yywy_ref < vtype > &v);
	inline vtype4(const vtype4_yywz_ref < vtype > &v);
	inline vtype4(const vtype4_yyww_ref < vtype > &v);
	inline vtype4(const vtype4_yzxx_ref < vtype > &v);
	inline vtype4(const vtype4_yzxy_ref < vtype > &v);
	inline vtype4(const vtype4_yzxz_ref < vtype > &v);
	inline vtype4(const vtype4_yzxw_ref < vtype > &v);
	inline vtype4(const vtype4_yzyx_ref < vtype > &v);
	inline vtype4(const vtype4_yzyy_ref < vtype > &v);
	inline vtype4(const vtype4_yzyz_ref < vtype > &v);
	inline vtype4(const vtype4_yzyw_ref < vtype > &v);
	inline vtype4(const vtype4_yzzx_ref < vtype > &v);
	inline vtype4(const vtype4_yzzy_ref < vtype > &v);
	inline vtype4(const vtype4_yzzz_ref < vtype > &v);
	inline vtype4(const vtype4_yzzw_ref < vtype > &v);
	inline vtype4(const vtype4_yzwx_ref < vtype > &v);
	inline vtype4(const vtype4_yzwy_ref < vtype > &v);
	inline vtype4(const vtype4_yzwz_ref < vtype > &v);
	inline vtype4(const vtype4_yzww_ref < vtype > &v);
	inline vtype4(const vtype4_ywxx_ref < vtype > &v);
	inline vtype4(const vtype4_ywxy_ref < vtype > &v);
	inline vtype4(const vtype4_ywxz_ref < vtype > &v);
	inline vtype4(const vtype4_ywxw_ref < vtype > &v);
	inline vtype4(const vtype4_ywyx_ref < vtype > &v);
	inline vtype4(const vtype4_ywyy_ref < vtype > &v);
	inline vtype4(const vtype4_ywyz_ref < vtype > &v);
	inline vtype4(const vtype4_ywyw_ref < vtype > &v);
	inline vtype4(const vtype4_ywzx_ref < vtype > &v);
	inline vtype4(const vtype4_ywzy_ref < vtype > &v);
	inline vtype4(const vtype4_ywzz_ref < vtype > &v);
	inline vtype4(const vtype4_ywzw_ref < vtype > &v);
	inline vtype4(const vtype4_ywwx_ref < vtype > &v);
	inline vtype4(const vtype4_ywwy_ref < vtype > &v);
	inline vtype4(const vtype4_ywwz_ref < vtype > &v);
	inline vtype4(const vtype4_ywww_ref < vtype > &v);
	inline vtype4(const vtype4_zxxx_ref < vtype > &v);
	inline vtype4(const vtype4_zxxy_ref < vtype > &v);
	inline vtype4(const vtype4_zxxz_ref < vtype > &v);
	inline vtype4(const vtype4_zxxw_ref < vtype > &v);
	inline vtype4(const vtype4_zxyx_ref < vtype > &v);
	inline vtype4(const vtype4_zxyy_ref < vtype > &v);
	inline vtype4(const vtype4_zxyz_ref < vtype > &v);
	inline vtype4(const vtype4_zxyw_ref < vtype > &v);
	inline vtype4(const vtype4_zxzx_ref < vtype > &v);
	inline vtype4(const vtype4_zxzy_ref < vtype > &v);
	inline vtype4(const vtype4_zxzz_ref < vtype > &v);
	inline vtype4(const vtype4_zxzw_ref < vtype > &v);
	inline vtype4(const vtype4_zxwx_ref < vtype > &v);
	inline vtype4(const vtype4_zxwy_ref < vtype > &v);
	inline vtype4(const vtype4_zxwz_ref < vtype > &v);
	inline vtype4(const vtype4_zxww_ref < vtype > &v);
	inline vtype4(const vtype4_zyxx_ref < vtype > &v);
	inline vtype4(const vtype4_zyxy_ref < vtype > &v);
	inline vtype4(const vtype4_zyxz_ref < vtype > &v);
	inline vtype4(const vtype4_zyxw_ref < vtype > &v);
	inline vtype4(const vtype4_zyyx_ref < vtype > &v);
	inline vtype4(const vtype4_zyyy_ref < vtype > &v);
	inline vtype4(const vtype4_zyyz_ref < vtype > &v);
	inline vtype4(const vtype4_zyyw_ref < vtype > &v);
	inline vtype4(const vtype4_zyzx_ref < vtype > &v);
	inline vtype4(const vtype4_zyzy_ref < vtype > &v);
	inline vtype4(const vtype4_zyzz_ref < vtype > &v);
	inline vtype4(const vtype4_zyzw_ref < vtype > &v);
	inline vtype4(const vtype4_zywx_ref < vtype > &v);
	inline vtype4(const vtype4_zywy_ref < vtype > &v);
	inline vtype4(const vtype4_zywz_ref < vtype > &v);
	inline vtype4(const vtype4_zyww_ref < vtype > &v);
	inline vtype4(const vtype4_zzxx_ref < vtype > &v);
	inline vtype4(const vtype4_zzxy_ref < vtype > &v);
	inline vtype4(const vtype4_zzxz_ref < vtype > &v);
	inline vtype4(const vtype4_zzxw_ref < vtype > &v);
	inline vtype4(const vtype4_zzyx_ref < vtype > &v);
	inline vtype4(const vtype4_zzyy_ref < vtype > &v);
	inline vtype4(const vtype4_zzyz_ref < vtype > &v);
	inline vtype4(const vtype4_zzyw_ref < vtype > &v);
	inline vtype4(const vtype4_zzzx_ref < vtype > &v);
	inline vtype4(const vtype4_zzzy_ref < vtype > &v);
	inline vtype4(const vtype4_zzzz_ref < vtype > &v);
	inline vtype4(const vtype4_zzzw_ref < vtype > &v);
	inline vtype4(const vtype4_zzwx_ref < vtype > &v);
	inline vtype4(const vtype4_zzwy_ref < vtype > &v);
	inline vtype4(const vtype4_zzwz_ref < vtype > &v);
	inline vtype4(const vtype4_zzww_ref < vtype > &v);
	inline vtype4(const vtype4_zwxx_ref < vtype > &v);
	inline vtype4(const vtype4_zwxy_ref < vtype > &v);
	inline vtype4(const vtype4_zwxz_ref < vtype > &v);
	inline vtype4(const vtype4_zwxw_ref < vtype > &v);
	inline vtype4(const vtype4_zwyx_ref < vtype > &v);
	inline vtype4(const vtype4_zwyy_ref < vtype > &v);
	inline vtype4(const vtype4_zwyz_ref < vtype > &v);
	inline vtype4(const vtype4_zwyw_ref < vtype > &v);
	inline vtype4(const vtype4_zwzx_ref < vtype > &v);
	inline vtype4(const vtype4_zwzy_ref < vtype > &v);
	inline vtype4(const vtype4_zwzz_ref < vtype > &v);
	inline vtype4(const vtype4_zwzw_ref < vtype > &v);
	inline vtype4(const vtype4_zwwx_ref < vtype > &v);
	inline vtype4(const vtype4_zwwy_ref < vtype > &v);
	inline vtype4(const vtype4_zwwz_ref < vtype > &v);
	inline vtype4(const vtype4_zwww_ref < vtype > &v);
	inline vtype4(const vtype4_wxxx_ref < vtype > &v);
	inline vtype4(const vtype4_wxxy_ref < vtype > &v);
	inline vtype4(const vtype4_wxxz_ref < vtype > &v);
	inline vtype4(const vtype4_wxxw_ref < vtype > &v);
	inline vtype4(const vtype4_wxyx_ref < vtype > &v);
	inline vtype4(const vtype4_wxyy_ref < vtype > &v);
	inline vtype4(const vtype4_wxyz_ref < vtype > &v);
	inline vtype4(const vtype4_wxyw_ref < vtype > &v);
	inline vtype4(const vtype4_wxzx_ref < vtype > &v);
	inline vtype4(const vtype4_wxzy_ref < vtype > &v);
	inline vtype4(const vtype4_wxzz_ref < vtype > &v);
	inline vtype4(const vtype4_wxzw_ref < vtype > &v);
	inline vtype4(const vtype4_wxwx_ref < vtype > &v);
	inline vtype4(const vtype4_wxwy_ref < vtype > &v);
	inline vtype4(const vtype4_wxwz_ref < vtype > &v);
	inline vtype4(const vtype4_wxww_ref < vtype > &v);
	inline vtype4(const vtype4_wyxx_ref < vtype > &v);
	inline vtype4(const vtype4_wyxy_ref < vtype > &v);
	inline vtype4(const vtype4_wyxz_ref < vtype > &v);
	inline vtype4(const vtype4_wyxw_ref < vtype > &v);
	inline vtype4(const vtype4_wyyx_ref < vtype > &v);
	inline vtype4(const vtype4_wyyy_ref < vtype > &v);
	inline vtype4(const vtype4_wyyz_ref < vtype > &v);
	inline vtype4(const vtype4_wyyw_ref < vtype > &v);
	inline vtype4(const vtype4_wyzx_ref < vtype > &v);
	inline vtype4(const vtype4_wyzy_ref < vtype > &v);
	inline vtype4(const vtype4_wyzz_ref < vtype > &v);
	inline vtype4(const vtype4_wyzw_ref < vtype > &v);
	inline vtype4(const vtype4_wywx_ref < vtype > &v);
	inline vtype4(const vtype4_wywy_ref < vtype > &v);
	inline vtype4(const vtype4_wywz_ref < vtype > &v);
	inline vtype4(const vtype4_wyww_ref < vtype > &v);
	inline vtype4(const vtype4_wzxx_ref < vtype > &v);
	inline vtype4(const vtype4_wzxy_ref < vtype > &v);
	inline vtype4(const vtype4_wzxz_ref < vtype > &v);
	inline vtype4(const vtype4_wzxw_ref < vtype > &v);
	inline vtype4(const vtype4_wzyx_ref < vtype > &v);
	inline vtype4(const vtype4_wzyy_ref < vtype > &v);
	inline vtype4(const vtype4_wzyz_ref < vtype > &v);
	inline vtype4(const vtype4_wzyw_ref < vtype > &v);
	inline vtype4(const vtype4_wzzx_ref < vtype > &v);
	inline vtype4(const vtype4_wzzy_ref < vtype > &v);
	inline vtype4(const vtype4_wzzz_ref < vtype > &v);
	inline vtype4(const vtype4_wzzw_ref < vtype > &v);
	inline vtype4(const vtype4_wzwx_ref < vtype > &v);
	inline vtype4(const vtype4_wzwy_ref < vtype > &v);
	inline vtype4(const vtype4_wzwz_ref < vtype > &v);
	inline vtype4(const vtype4_wzww_ref < vtype > &v);
	inline vtype4(const vtype4_wwxx_ref < vtype > &v);
	inline vtype4(const vtype4_wwxy_ref < vtype > &v);
	inline vtype4(const vtype4_wwxz_ref < vtype > &v);
	inline vtype4(const vtype4_wwxw_ref < vtype > &v);
	inline vtype4(const vtype4_wwyx_ref < vtype > &v);
	inline vtype4(const vtype4_wwyy_ref < vtype > &v);
	inline vtype4(const vtype4_wwyz_ref < vtype > &v);
	inline vtype4(const vtype4_wwyw_ref < vtype > &v);
	inline vtype4(const vtype4_wwzx_ref < vtype > &v);
	inline vtype4(const vtype4_wwzy_ref < vtype > &v);
	inline vtype4(const vtype4_wwzz_ref < vtype > &v);
	inline vtype4(const vtype4_wwzw_ref < vtype > &v);
	inline vtype4(const vtype4_wwwx_ref < vtype > &v);
	inline vtype4(const vtype4_wwwy_ref < vtype > &v);
	inline vtype4(const vtype4_wwwz_ref < vtype > &v);
	inline vtype4(const vtype4_wwww_ref < vtype > &v);
	vtype4_xx_ref < vtype > xx()
	{
		return vtype4_xx_ref < vtype > (this);
	}
	vtype4_xy_ref < vtype > xy()
	{
		return vtype4_xy_ref < vtype > (this);
	}
	vtype4_xz_ref < vtype > xz()
	{
		return vtype4_xz_ref < vtype > (this);
	}
	vtype4_xw_ref < vtype > xw()
	{
		return vtype4_xw_ref < vtype > (this);
	}
	vtype4_yx_ref < vtype > yx()
	{
		return vtype4_yx_ref < vtype > (this);
	}
	vtype4_yy_ref < vtype > yy()
	{
		return vtype4_yy_ref < vtype > (this);
	}
	vtype4_yz_ref < vtype > yz()
	{
		return vtype4_yz_ref < vtype > (this);
	}
	vtype4_yw_ref < vtype > yw()
	{
		return vtype4_yw_ref < vtype > (this);
	}
	vtype4_zx_ref < vtype > zx()
	{
		return vtype4_zx_ref < vtype > (this);
	}
	vtype4_zy_ref < vtype > zy()
	{
		return vtype4_zy_ref < vtype > (this);
	}
	vtype4_zz_ref < vtype > zz()
	{
		return vtype4_zz_ref < vtype > (this);
	}
	vtype4_zw_ref < vtype > zw()
	{
		return vtype4_zw_ref < vtype > (this);
	}
	vtype4_wx_ref < vtype > wx()
	{
		return vtype4_wx_ref < vtype > (this);
	}
	vtype4_wy_ref < vtype > wy()
	{
		return vtype4_wy_ref < vtype > (this);
	}
	vtype4_wz_ref < vtype > wz()
	{
		return vtype4_wz_ref < vtype > (this);
	}
	vtype4_ww_ref < vtype > ww()
	{
		return vtype4_ww_ref < vtype > (this);
	}
	vtype4_xxx_ref < vtype > xxx()
	{
		return vtype4_xxx_ref < vtype > (this);
	}
	vtype4_xxy_ref < vtype > xxy()
	{
		return vtype4_xxy_ref < vtype > (this);
	}
	vtype4_xxz_ref < vtype > xxz()
	{
		return vtype4_xxz_ref < vtype > (this);
	}
	vtype4_xxw_ref < vtype > xxw()
	{
		return vtype4_xxw_ref < vtype > (this);
	}
	vtype4_xyx_ref < vtype > xyx()
	{
		return vtype4_xyx_ref < vtype > (this);
	}
	vtype4_xyy_ref < vtype > xyy()
	{
		return vtype4_xyy_ref < vtype > (this);
	}
	vtype4_xyz_ref < vtype > xyz()
	{
		return vtype4_xyz_ref < vtype > (this);
	}
	vtype4_xyw_ref < vtype > xyw()
	{
		return vtype4_xyw_ref < vtype > (this);
	}
	vtype4_xzx_ref < vtype > xzx()
	{
		return vtype4_xzx_ref < vtype > (this);
	}
	vtype4_xzy_ref < vtype > xzy()
	{
		return vtype4_xzy_ref < vtype > (this);
	}
	vtype4_xzz_ref < vtype > xzz()
	{
		return vtype4_xzz_ref < vtype > (this);
	}
	vtype4_xzw_ref < vtype > xzw()
	{
		return vtype4_xzw_ref < vtype > (this);
	}
	vtype4_xwx_ref < vtype > xwx()
	{
		return vtype4_xwx_ref < vtype > (this);
	}
	vtype4_xwy_ref < vtype > xwy()
	{
		return vtype4_xwy_ref < vtype > (this);
	}
	vtype4_xwz_ref < vtype > xwz()
	{
		return vtype4_xwz_ref < vtype > (this);
	}
	vtype4_xww_ref < vtype > xww()
	{
		return vtype4_xww_ref < vtype > (this);
	}
	vtype4_yxx_ref < vtype > yxx()
	{
		return vtype4_yxx_ref < vtype > (this);
	}
	vtype4_yxy_ref < vtype > yxy()
	{
		return vtype4_yxy_ref < vtype > (this);
	}
	vtype4_yxz_ref < vtype > yxz()
	{
		return vtype4_yxz_ref < vtype > (this);
	}
	vtype4_yxw_ref < vtype > yxw()
	{
		return vtype4_yxw_ref < vtype > (this);
	}
	vtype4_yyx_ref < vtype > yyx()
	{
		return vtype4_yyx_ref < vtype > (this);
	}
	vtype4_yyy_ref < vtype > yyy()
	{
		return vtype4_yyy_ref < vtype > (this);
	}
	vtype4_yyz_ref < vtype > yyz()
	{
		return vtype4_yyz_ref < vtype > (this);
	}
	vtype4_yyw_ref < vtype > yyw()
	{
		return vtype4_yyw_ref < vtype > (this);
	}
	vtype4_yzx_ref < vtype > yzx()
	{
		return vtype4_yzx_ref < vtype > (this);
	}
	vtype4_yzy_ref < vtype > yzy()
	{
		return vtype4_yzy_ref < vtype > (this);
	}
	vtype4_yzz_ref < vtype > yzz()
	{
		return vtype4_yzz_ref < vtype > (this);
	}
	vtype4_yzw_ref < vtype > yzw()
	{
		return vtype4_yzw_ref < vtype > (this);
	}
	vtype4_ywx_ref < vtype > ywx()
	{
		return vtype4_ywx_ref < vtype > (this);
	}
	vtype4_ywy_ref < vtype > ywy()
	{
		return vtype4_ywy_ref < vtype > (this);
	}
	vtype4_ywz_ref < vtype > ywz()
	{
		return vtype4_ywz_ref < vtype > (this);
	}
	vtype4_yww_ref < vtype > yww()
	{
		return vtype4_yww_ref < vtype > (this);
	}
	vtype4_zxx_ref < vtype > zxx()
	{
		return vtype4_zxx_ref < vtype > (this);
	}
	vtype4_zxy_ref < vtype > zxy()
	{
		return vtype4_zxy_ref < vtype > (this);
	}
	vtype4_zxz_ref < vtype > zxz()
	{
		return vtype4_zxz_ref < vtype > (this);
	}
	vtype4_zxw_ref < vtype > zxw()
	{
		return vtype4_zxw_ref < vtype > (this);
	}
	vtype4_zyx_ref < vtype > zyx()
	{
		return vtype4_zyx_ref < vtype > (this);
	}
	vtype4_zyy_ref < vtype > zyy()
	{
		return vtype4_zyy_ref < vtype > (this);
	}
	vtype4_zyz_ref < vtype > zyz()
	{
		return vtype4_zyz_ref < vtype > (this);
	}
	vtype4_zyw_ref < vtype > zyw()
	{
		return vtype4_zyw_ref < vtype > (this);
	}
	vtype4_zzx_ref < vtype > zzx()
	{
		return vtype4_zzx_ref < vtype > (this);
	}
	vtype4_zzy_ref < vtype > zzy()
	{
		return vtype4_zzy_ref < vtype > (this);
	}
	vtype4_zzz_ref < vtype > zzz()
	{
		return vtype4_zzz_ref < vtype > (this);
	}
	vtype4_zzw_ref < vtype > zzw()
	{
		return vtype4_zzw_ref < vtype > (this);
	}
	vtype4_zwx_ref < vtype > zwx()
	{
		return vtype4_zwx_ref < vtype > (this);
	}
	vtype4_zwy_ref < vtype > zwy()
	{
		return vtype4_zwy_ref < vtype > (this);
	}
	vtype4_zwz_ref < vtype > zwz()
	{
		return vtype4_zwz_ref < vtype > (this);
	}
	vtype4_zww_ref < vtype > zww()
	{
		return vtype4_zww_ref < vtype > (this);
	}
	vtype4_wxx_ref < vtype > wxx()
	{
		return vtype4_wxx_ref < vtype > (this);
	}
	vtype4_wxy_ref < vtype > wxy()
	{
		return vtype4_wxy_ref < vtype > (this);
	}
	vtype4_wxz_ref < vtype > wxz()
	{
		return vtype4_wxz_ref < vtype > (this);
	}
	vtype4_wxw_ref < vtype > wxw()
	{
		return vtype4_wxw_ref < vtype > (this);
	}
	vtype4_wyx_ref < vtype > wyx()
	{
		return vtype4_wyx_ref < vtype > (this);
	}
	vtype4_wyy_ref < vtype > wyy()
	{
		return vtype4_wyy_ref < vtype > (this);
	}
	vtype4_wyz_ref < vtype > wyz()
	{
		return vtype4_wyz_ref < vtype > (this);
	}
	vtype4_wyw_ref < vtype > wyw()
	{
		return vtype4_wyw_ref < vtype > (this);
	}
	vtype4_wzx_ref < vtype > wzx()
	{
		return vtype4_wzx_ref < vtype > (this);
	}
	vtype4_wzy_ref < vtype > wzy()
	{
		return vtype4_wzy_ref < vtype > (this);
	}
	vtype4_wzz_ref < vtype > wzz()
	{
		return vtype4_wzz_ref < vtype > (this);
	}
	vtype4_wzw_ref < vtype > wzw()
	{
		return vtype4_wzw_ref < vtype > (this);
	}
	vtype4_wwx_ref < vtype > wwx()
	{
		return vtype4_wwx_ref < vtype > (this);
	}
	vtype4_wwy_ref < vtype > wwy()
	{
		return vtype4_wwy_ref < vtype > (this);
	}
	vtype4_wwz_ref < vtype > wwz()
	{
		return vtype4_wwz_ref < vtype > (this);
	}
	vtype4_www_ref < vtype > www()
	{
		return vtype4_www_ref < vtype > (this);
	}
	vtype4_xxxx_ref < vtype > xxxx()
	{
		return vtype4_xxxx_ref < vtype > (this);
	}
	vtype4_xxxy_ref < vtype > xxxy()
	{
		return vtype4_xxxy_ref < vtype > (this);
	}
	vtype4_xxxz_ref < vtype > xxxz()
	{
		return vtype4_xxxz_ref < vtype > (this);
	}
	vtype4_xxxw_ref < vtype > xxxw()
	{
		return vtype4_xxxw_ref < vtype > (this);
	}
	vtype4_xxyx_ref < vtype > xxyx()
	{
		return vtype4_xxyx_ref < vtype > (this);
	}
	vtype4_xxyy_ref < vtype > xxyy()
	{
		return vtype4_xxyy_ref < vtype > (this);
	}
	vtype4_xxyz_ref < vtype > xxyz()
	{
		return vtype4_xxyz_ref < vtype > (this);
	}
	vtype4_xxyw_ref < vtype > xxyw()
	{
		return vtype4_xxyw_ref < vtype > (this);
	}
	vtype4_xxzx_ref < vtype > xxzx()
	{
		return vtype4_xxzx_ref < vtype > (this);
	}
	vtype4_xxzy_ref < vtype > xxzy()
	{
		return vtype4_xxzy_ref < vtype > (this);
	}
	vtype4_xxzz_ref < vtype > xxzz()
	{
		return vtype4_xxzz_ref < vtype > (this);
	}
	vtype4_xxzw_ref < vtype > xxzw()
	{
		return vtype4_xxzw_ref < vtype > (this);
	}
	vtype4_xxwx_ref < vtype > xxwx()
	{
		return vtype4_xxwx_ref < vtype > (this);
	}
	vtype4_xxwy_ref < vtype > xxwy()
	{
		return vtype4_xxwy_ref < vtype > (this);
	}
	vtype4_xxwz_ref < vtype > xxwz()
	{
		return vtype4_xxwz_ref < vtype > (this);
	}
	vtype4_xxww_ref < vtype > xxww()
	{
		return vtype4_xxww_ref < vtype > (this);
	}
	vtype4_xyxx_ref < vtype > xyxx()
	{
		return vtype4_xyxx_ref < vtype > (this);
	}
	vtype4_xyxy_ref < vtype > xyxy()
	{
		return vtype4_xyxy_ref < vtype > (this);
	}
	vtype4_xyxz_ref < vtype > xyxz()
	{
		return vtype4_xyxz_ref < vtype > (this);
	}
	vtype4_xyxw_ref < vtype > xyxw()
	{
		return vtype4_xyxw_ref < vtype > (this);
	}
	vtype4_xyyx_ref < vtype > xyyx()
	{
		return vtype4_xyyx_ref < vtype > (this);
	}
	vtype4_xyyy_ref < vtype > xyyy()
	{
		return vtype4_xyyy_ref < vtype > (this);
	}
	vtype4_xyyz_ref < vtype > xyyz()
	{
		return vtype4_xyyz_ref < vtype > (this);
	}
	vtype4_xyyw_ref < vtype > xyyw()
	{
		return vtype4_xyyw_ref < vtype > (this);
	}
	vtype4_xyzx_ref < vtype > xyzx()
	{
		return vtype4_xyzx_ref < vtype > (this);
	}
	vtype4_xyzy_ref < vtype > xyzy()
	{
		return vtype4_xyzy_ref < vtype > (this);
	}
	vtype4_xyzz_ref < vtype > xyzz()
	{
		return vtype4_xyzz_ref < vtype > (this);
	}
	vtype4_xyzw_ref < vtype > xyzw()
	{
		return vtype4_xyzw_ref < vtype > (this);
	}
	vtype4_xywx_ref < vtype > xywx()
	{
		return vtype4_xywx_ref < vtype > (this);
	}
	vtype4_xywy_ref < vtype > xywy()
	{
		return vtype4_xywy_ref < vtype > (this);
	}
	vtype4_xywz_ref < vtype > xywz()
	{
		return vtype4_xywz_ref < vtype > (this);
	}
	vtype4_xyww_ref < vtype > xyww()
	{
		return vtype4_xyww_ref < vtype > (this);
	}
	vtype4_xzxx_ref < vtype > xzxx()
	{
		return vtype4_xzxx_ref < vtype > (this);
	}
	vtype4_xzxy_ref < vtype > xzxy()
	{
		return vtype4_xzxy_ref < vtype > (this);
	}
	vtype4_xzxz_ref < vtype > xzxz()
	{
		return vtype4_xzxz_ref < vtype > (this);
	}
	vtype4_xzxw_ref < vtype > xzxw()
	{
		return vtype4_xzxw_ref < vtype > (this);
	}
	vtype4_xzyx_ref < vtype > xzyx()
	{
		return vtype4_xzyx_ref < vtype > (this);
	}
	vtype4_xzyy_ref < vtype > xzyy()
	{
		return vtype4_xzyy_ref < vtype > (this);
	}
	vtype4_xzyz_ref < vtype > xzyz()
	{
		return vtype4_xzyz_ref < vtype > (this);
	}
	vtype4_xzyw_ref < vtype > xzyw()
	{
		return vtype4_xzyw_ref < vtype > (this);
	}
	vtype4_xzzx_ref < vtype > xzzx()
	{
		return vtype4_xzzx_ref < vtype > (this);
	}
	vtype4_xzzy_ref < vtype > xzzy()
	{
		return vtype4_xzzy_ref < vtype > (this);
	}
	vtype4_xzzz_ref < vtype > xzzz()
	{
		return vtype4_xzzz_ref < vtype > (this);
	}
	vtype4_xzzw_ref < vtype > xzzw()
	{
		return vtype4_xzzw_ref < vtype > (this);
	}
	vtype4_xzwx_ref < vtype > xzwx()
	{
		return vtype4_xzwx_ref < vtype > (this);
	}
	vtype4_xzwy_ref < vtype > xzwy()
	{
		return vtype4_xzwy_ref < vtype > (this);
	}
	vtype4_xzwz_ref < vtype > xzwz()
	{
		return vtype4_xzwz_ref < vtype > (this);
	}
	vtype4_xzww_ref < vtype > xzww()
	{
		return vtype4_xzww_ref < vtype > (this);
	}
	vtype4_xwxx_ref < vtype > xwxx()
	{
		return vtype4_xwxx_ref < vtype > (this);
	}
	vtype4_xwxy_ref < vtype > xwxy()
	{
		return vtype4_xwxy_ref < vtype > (this);
	}
	vtype4_xwxz_ref < vtype > xwxz()
	{
		return vtype4_xwxz_ref < vtype > (this);
	}
	vtype4_xwxw_ref < vtype > xwxw()
	{
		return vtype4_xwxw_ref < vtype > (this);
	}
	vtype4_xwyx_ref < vtype > xwyx()
	{
		return vtype4_xwyx_ref < vtype > (this);
	}
	vtype4_xwyy_ref < vtype > xwyy()
	{
		return vtype4_xwyy_ref < vtype > (this);
	}
	vtype4_xwyz_ref < vtype > xwyz()
	{
		return vtype4_xwyz_ref < vtype > (this);
	}
	vtype4_xwyw_ref < vtype > xwyw()
	{
		return vtype4_xwyw_ref < vtype > (this);
	}
	vtype4_xwzx_ref < vtype > xwzx()
	{
		return vtype4_xwzx_ref < vtype > (this);
	}
	vtype4_xwzy_ref < vtype > xwzy()
	{
		return vtype4_xwzy_ref < vtype > (this);
	}
	vtype4_xwzz_ref < vtype > xwzz()
	{
		return vtype4_xwzz_ref < vtype > (this);
	}
	vtype4_xwzw_ref < vtype > xwzw()
	{
		return vtype4_xwzw_ref < vtype > (this);
	}
	vtype4_xwwx_ref < vtype > xwwx()
	{
		return vtype4_xwwx_ref < vtype > (this);
	}
	vtype4_xwwy_ref < vtype > xwwy()
	{
		return vtype4_xwwy_ref < vtype > (this);
	}
	vtype4_xwwz_ref < vtype > xwwz()
	{
		return vtype4_xwwz_ref < vtype > (this);
	}
	vtype4_xwww_ref < vtype > xwww()
	{
		return vtype4_xwww_ref < vtype > (this);
	}
	vtype4_yxxx_ref < vtype > yxxx()
	{
		return vtype4_yxxx_ref < vtype > (this);
	}
	vtype4_yxxy_ref < vtype > yxxy()
	{
		return vtype4_yxxy_ref < vtype > (this);
	}
	vtype4_yxxz_ref < vtype > yxxz()
	{
		return vtype4_yxxz_ref < vtype > (this);
	}
	vtype4_yxxw_ref < vtype > yxxw()
	{
		return vtype4_yxxw_ref < vtype > (this);
	}
	vtype4_yxyx_ref < vtype > yxyx()
	{
		return vtype4_yxyx_ref < vtype > (this);
	}
	vtype4_yxyy_ref < vtype > yxyy()
	{
		return vtype4_yxyy_ref < vtype > (this);
	}
	vtype4_yxyz_ref < vtype > yxyz()
	{
		return vtype4_yxyz_ref < vtype > (this);
	}
	vtype4_yxyw_ref < vtype > yxyw()
	{
		return vtype4_yxyw_ref < vtype > (this);
	}
	vtype4_yxzx_ref < vtype > yxzx()
	{
		return vtype4_yxzx_ref < vtype > (this);
	}
	vtype4_yxzy_ref < vtype > yxzy()
	{
		return vtype4_yxzy_ref < vtype > (this);
	}
	vtype4_yxzz_ref < vtype > yxzz()
	{
		return vtype4_yxzz_ref < vtype > (this);
	}
	vtype4_yxzw_ref < vtype > yxzw()
	{
		return vtype4_yxzw_ref < vtype > (this);
	}
	vtype4_yxwx_ref < vtype > yxwx()
	{
		return vtype4_yxwx_ref < vtype > (this);
	}
	vtype4_yxwy_ref < vtype > yxwy()
	{
		return vtype4_yxwy_ref < vtype > (this);
	}
	vtype4_yxwz_ref < vtype > yxwz()
	{
		return vtype4_yxwz_ref < vtype > (this);
	}
	vtype4_yxww_ref < vtype > yxww()
	{
		return vtype4_yxww_ref < vtype > (this);
	}
	vtype4_yyxx_ref < vtype > yyxx()
	{
		return vtype4_yyxx_ref < vtype > (this);
	}
	vtype4_yyxy_ref < vtype > yyxy()
	{
		return vtype4_yyxy_ref < vtype > (this);
	}
	vtype4_yyxz_ref < vtype > yyxz()
	{
		return vtype4_yyxz_ref < vtype > (this);
	}
	vtype4_yyxw_ref < vtype > yyxw()
	{
		return vtype4_yyxw_ref < vtype > (this);
	}
	vtype4_yyyx_ref < vtype > yyyx()
	{
		return vtype4_yyyx_ref < vtype > (this);
	}
	vtype4_yyyy_ref < vtype > yyyy()
	{
		return vtype4_yyyy_ref < vtype > (this);
	}
	vtype4_yyyz_ref < vtype > yyyz()
	{
		return vtype4_yyyz_ref < vtype > (this);
	}
	vtype4_yyyw_ref < vtype > yyyw()
	{
		return vtype4_yyyw_ref < vtype > (this);
	}
	vtype4_yyzx_ref < vtype > yyzx()
	{
		return vtype4_yyzx_ref < vtype > (this);
	}
	vtype4_yyzy_ref < vtype > yyzy()
	{
		return vtype4_yyzy_ref < vtype > (this);
	}
	vtype4_yyzz_ref < vtype > yyzz()
	{
		return vtype4_yyzz_ref < vtype > (this);
	}
	vtype4_yyzw_ref < vtype > yyzw()
	{
		return vtype4_yyzw_ref < vtype > (this);
	}
	vtype4_yywx_ref < vtype > yywx()
	{
		return vtype4_yywx_ref < vtype > (this);
	}
	vtype4_yywy_ref < vtype > yywy()
	{
		return vtype4_yywy_ref < vtype > (this);
	}
	vtype4_yywz_ref < vtype > yywz()
	{
		return vtype4_yywz_ref < vtype > (this);
	}
	vtype4_yyww_ref < vtype > yyww()
	{
		return vtype4_yyww_ref < vtype > (this);
	}
	vtype4_yzxx_ref < vtype > yzxx()
	{
		return vtype4_yzxx_ref < vtype > (this);
	}
	vtype4_yzxy_ref < vtype > yzxy()
	{
		return vtype4_yzxy_ref < vtype > (this);
	}
	vtype4_yzxz_ref < vtype > yzxz()
	{
		return vtype4_yzxz_ref < vtype > (this);
	}
	vtype4_yzxw_ref < vtype > yzxw()
	{
		return vtype4_yzxw_ref < vtype > (this);
	}
	vtype4_yzyx_ref < vtype > yzyx()
	{
		return vtype4_yzyx_ref < vtype > (this);
	}
	vtype4_yzyy_ref < vtype > yzyy()
	{
		return vtype4_yzyy_ref < vtype > (this);
	}
	vtype4_yzyz_ref < vtype > yzyz()
	{
		return vtype4_yzyz_ref < vtype > (this);
	}
	vtype4_yzyw_ref < vtype > yzyw()
	{
		return vtype4_yzyw_ref < vtype > (this);
	}
	vtype4_yzzx_ref < vtype > yzzx()
	{
		return vtype4_yzzx_ref < vtype > (this);
	}
	vtype4_yzzy_ref < vtype > yzzy()
	{
		return vtype4_yzzy_ref < vtype > (this);
	}
	vtype4_yzzz_ref < vtype > yzzz()
	{
		return vtype4_yzzz_ref < vtype > (this);
	}
	vtype4_yzzw_ref < vtype > yzzw()
	{
		return vtype4_yzzw_ref < vtype > (this);
	}
	vtype4_yzwx_ref < vtype > yzwx()
	{
		return vtype4_yzwx_ref < vtype > (this);
	}
	vtype4_yzwy_ref < vtype > yzwy()
	{
		return vtype4_yzwy_ref < vtype > (this);
	}
	vtype4_yzwz_ref < vtype > yzwz()
	{
		return vtype4_yzwz_ref < vtype > (this);
	}
	vtype4_yzww_ref < vtype > yzww()
	{
		return vtype4_yzww_ref < vtype > (this);
	}
	vtype4_ywxx_ref < vtype > ywxx()
	{
		return vtype4_ywxx_ref < vtype > (this);
	}
	vtype4_ywxy_ref < vtype > ywxy()
	{
		return vtype4_ywxy_ref < vtype > (this);
	}
	vtype4_ywxz_ref < vtype > ywxz()
	{
		return vtype4_ywxz_ref < vtype > (this);
	}
	vtype4_ywxw_ref < vtype > ywxw()
	{
		return vtype4_ywxw_ref < vtype > (this);
	}
	vtype4_ywyx_ref < vtype > ywyx()
	{
		return vtype4_ywyx_ref < vtype > (this);
	}
	vtype4_ywyy_ref < vtype > ywyy()
	{
		return vtype4_ywyy_ref < vtype > (this);
	}
	vtype4_ywyz_ref < vtype > ywyz()
	{
		return vtype4_ywyz_ref < vtype > (this);
	}
	vtype4_ywyw_ref < vtype > ywyw()
	{
		return vtype4_ywyw_ref < vtype > (this);
	}
	vtype4_ywzx_ref < vtype > ywzx()
	{
		return vtype4_ywzx_ref < vtype > (this);
	}
	vtype4_ywzy_ref < vtype > ywzy()
	{
		return vtype4_ywzy_ref < vtype > (this);
	}
	vtype4_ywzz_ref < vtype > ywzz()
	{
		return vtype4_ywzz_ref < vtype > (this);
	}
	vtype4_ywzw_ref < vtype > ywzw()
	{
		return vtype4_ywzw_ref < vtype > (this);
	}
	vtype4_ywwx_ref < vtype > ywwx()
	{
		return vtype4_ywwx_ref < vtype > (this);
	}
	vtype4_ywwy_ref < vtype > ywwy()
	{
		return vtype4_ywwy_ref < vtype > (this);
	}
	vtype4_ywwz_ref < vtype > ywwz()
	{
		return vtype4_ywwz_ref < vtype > (this);
	}
	vtype4_ywww_ref < vtype > ywww()
	{
		return vtype4_ywww_ref < vtype > (this);
	}
	vtype4_zxxx_ref < vtype > zxxx()
	{
		return vtype4_zxxx_ref < vtype > (this);
	}
	vtype4_zxxy_ref < vtype > zxxy()
	{
		return vtype4_zxxy_ref < vtype > (this);
	}
	vtype4_zxxz_ref < vtype > zxxz()
	{
		return vtype4_zxxz_ref < vtype > (this);
	}
	vtype4_zxxw_ref < vtype > zxxw()
	{
		return vtype4_zxxw_ref < vtype > (this);
	}
	vtype4_zxyx_ref < vtype > zxyx()
	{
		return vtype4_zxyx_ref < vtype > (this);
	}
	vtype4_zxyy_ref < vtype > zxyy()
	{
		return vtype4_zxyy_ref < vtype > (this);
	}
	vtype4_zxyz_ref < vtype > zxyz()
	{
		return vtype4_zxyz_ref < vtype > (this);
	}
	vtype4_zxyw_ref < vtype > zxyw()
	{
		return vtype4_zxyw_ref < vtype > (this);
	}
	vtype4_zxzx_ref < vtype > zxzx()
	{
		return vtype4_zxzx_ref < vtype > (this);
	}
	vtype4_zxzy_ref < vtype > zxzy()
	{
		return vtype4_zxzy_ref < vtype > (this);
	}
	vtype4_zxzz_ref < vtype > zxzz()
	{
		return vtype4_zxzz_ref < vtype > (this);
	}
	vtype4_zxzw_ref < vtype > zxzw()
	{
		return vtype4_zxzw_ref < vtype > (this);
	}
	vtype4_zxwx_ref < vtype > zxwx()
	{
		return vtype4_zxwx_ref < vtype > (this);
	}
	vtype4_zxwy_ref < vtype > zxwy()
	{
		return vtype4_zxwy_ref < vtype > (this);
	}
	vtype4_zxwz_ref < vtype > zxwz()
	{
		return vtype4_zxwz_ref < vtype > (this);
	}
	vtype4_zxww_ref < vtype > zxww()
	{
		return vtype4_zxww_ref < vtype > (this);
	}
	vtype4_zyxx_ref < vtype > zyxx()
	{
		return vtype4_zyxx_ref < vtype > (this);
	}
	vtype4_zyxy_ref < vtype > zyxy()
	{
		return vtype4_zyxy_ref < vtype > (this);
	}
	vtype4_zyxz_ref < vtype > zyxz()
	{
		return vtype4_zyxz_ref < vtype > (this);
	}
	vtype4_zyxw_ref < vtype > zyxw()
	{
		return vtype4_zyxw_ref < vtype > (this);
	}
	vtype4_zyyx_ref < vtype > zyyx()
	{
		return vtype4_zyyx_ref < vtype > (this);
	}
	vtype4_zyyy_ref < vtype > zyyy()
	{
		return vtype4_zyyy_ref < vtype > (this);
	}
	vtype4_zyyz_ref < vtype > zyyz()
	{
		return vtype4_zyyz_ref < vtype > (this);
	}
	vtype4_zyyw_ref < vtype > zyyw()
	{
		return vtype4_zyyw_ref < vtype > (this);
	}
	vtype4_zyzx_ref < vtype > zyzx()
	{
		return vtype4_zyzx_ref < vtype > (this);
	}
	vtype4_zyzy_ref < vtype > zyzy()
	{
		return vtype4_zyzy_ref < vtype > (this);
	}
	vtype4_zyzz_ref < vtype > zyzz()
	{
		return vtype4_zyzz_ref < vtype > (this);
	}
	vtype4_zyzw_ref < vtype > zyzw()
	{
		return vtype4_zyzw_ref < vtype > (this);
	}
	vtype4_zywx_ref < vtype > zywx()
	{
		return vtype4_zywx_ref < vtype > (this);
	}
	vtype4_zywy_ref < vtype > zywy()
	{
		return vtype4_zywy_ref < vtype > (this);
	}
	vtype4_zywz_ref < vtype > zywz()
	{
		return vtype4_zywz_ref < vtype > (this);
	}
	vtype4_zyww_ref < vtype > zyww()
	{
		return vtype4_zyww_ref < vtype > (this);
	}
	vtype4_zzxx_ref < vtype > zzxx()
	{
		return vtype4_zzxx_ref < vtype > (this);
	}
	vtype4_zzxy_ref < vtype > zzxy()
	{
		return vtype4_zzxy_ref < vtype > (this);
	}
	vtype4_zzxz_ref < vtype > zzxz()
	{
		return vtype4_zzxz_ref < vtype > (this);
	}
	vtype4_zzxw_ref < vtype > zzxw()
	{
		return vtype4_zzxw_ref < vtype > (this);
	}
	vtype4_zzyx_ref < vtype > zzyx()
	{
		return vtype4_zzyx_ref < vtype > (this);
	}
	vtype4_zzyy_ref < vtype > zzyy()
	{
		return vtype4_zzyy_ref < vtype > (this);
	}
	vtype4_zzyz_ref < vtype > zzyz()
	{
		return vtype4_zzyz_ref < vtype > (this);
	}
	vtype4_zzyw_ref < vtype > zzyw()
	{
		return vtype4_zzyw_ref < vtype > (this);
	}
	vtype4_zzzx_ref < vtype > zzzx()
	{
		return vtype4_zzzx_ref < vtype > (this);
	}
	vtype4_zzzy_ref < vtype > zzzy()
	{
		return vtype4_zzzy_ref < vtype > (this);
	}
	vtype4_zzzz_ref < vtype > zzzz()
	{
		return vtype4_zzzz_ref < vtype > (this);
	}
	vtype4_zzzw_ref < vtype > zzzw()
	{
		return vtype4_zzzw_ref < vtype > (this);
	}
	vtype4_zzwx_ref < vtype > zzwx()
	{
		return vtype4_zzwx_ref < vtype > (this);
	}
	vtype4_zzwy_ref < vtype > zzwy()
	{
		return vtype4_zzwy_ref < vtype > (this);
	}
	vtype4_zzwz_ref < vtype > zzwz()
	{
		return vtype4_zzwz_ref < vtype > (this);
	}
	vtype4_zzww_ref < vtype > zzww()
	{
		return vtype4_zzww_ref < vtype > (this);
	}
	vtype4_zwxx_ref < vtype > zwxx()
	{
		return vtype4_zwxx_ref < vtype > (this);
	}
	vtype4_zwxy_ref < vtype > zwxy()
	{
		return vtype4_zwxy_ref < vtype > (this);
	}
	vtype4_zwxz_ref < vtype > zwxz()
	{
		return vtype4_zwxz_ref < vtype > (this);
	}
	vtype4_zwxw_ref < vtype > zwxw()
	{
		return vtype4_zwxw_ref < vtype > (this);
	}
	vtype4_zwyx_ref < vtype > zwyx()
	{
		return vtype4_zwyx_ref < vtype > (this);
	}
	vtype4_zwyy_ref < vtype > zwyy()
	{
		return vtype4_zwyy_ref < vtype > (this);
	}
	vtype4_zwyz_ref < vtype > zwyz()
	{
		return vtype4_zwyz_ref < vtype > (this);
	}
	vtype4_zwyw_ref < vtype > zwyw()
	{
		return vtype4_zwyw_ref < vtype > (this);
	}
	vtype4_zwzx_ref < vtype > zwzx()
	{
		return vtype4_zwzx_ref < vtype > (this);
	}
	vtype4_zwzy_ref < vtype > zwzy()
	{
		return vtype4_zwzy_ref < vtype > (this);
	}
	vtype4_zwzz_ref < vtype > zwzz()
	{
		return vtype4_zwzz_ref < vtype > (this);
	}
	vtype4_zwzw_ref < vtype > zwzw()
	{
		return vtype4_zwzw_ref < vtype > (this);
	}
	vtype4_zwwx_ref < vtype > zwwx()
	{
		return vtype4_zwwx_ref < vtype > (this);
	}
	vtype4_zwwy_ref < vtype > zwwy()
	{
		return vtype4_zwwy_ref < vtype > (this);
	}
	vtype4_zwwz_ref < vtype > zwwz()
	{
		return vtype4_zwwz_ref < vtype > (this);
	}
	vtype4_zwww_ref < vtype > zwww()
	{
		return vtype4_zwww_ref < vtype > (this);
	}
	vtype4_wxxx_ref < vtype > wxxx()
	{
		return vtype4_wxxx_ref < vtype > (this);
	}
	vtype4_wxxy_ref < vtype > wxxy()
	{
		return vtype4_wxxy_ref < vtype > (this);
	}
	vtype4_wxxz_ref < vtype > wxxz()
	{
		return vtype4_wxxz_ref < vtype > (this);
	}
	vtype4_wxxw_ref < vtype > wxxw()
	{
		return vtype4_wxxw_ref < vtype > (this);
	}
	vtype4_wxyx_ref < vtype > wxyx()
	{
		return vtype4_wxyx_ref < vtype > (this);
	}
	vtype4_wxyy_ref < vtype > wxyy()
	{
		return vtype4_wxyy_ref < vtype > (this);
	}
	vtype4_wxyz_ref < vtype > wxyz()
	{
		return vtype4_wxyz_ref < vtype > (this);
	}
	vtype4_wxyw_ref < vtype > wxyw()
	{
		return vtype4_wxyw_ref < vtype > (this);
	}
	vtype4_wxzx_ref < vtype > wxzx()
	{
		return vtype4_wxzx_ref < vtype > (this);
	}
	vtype4_wxzy_ref < vtype > wxzy()
	{
		return vtype4_wxzy_ref < vtype > (this);
	}
	vtype4_wxzz_ref < vtype > wxzz()
	{
		return vtype4_wxzz_ref < vtype > (this);
	}
	vtype4_wxzw_ref < vtype > wxzw()
	{
		return vtype4_wxzw_ref < vtype > (this);
	}
	vtype4_wxwx_ref < vtype > wxwx()
	{
		return vtype4_wxwx_ref < vtype > (this);
	}
	vtype4_wxwy_ref < vtype > wxwy()
	{
		return vtype4_wxwy_ref < vtype > (this);
	}
	vtype4_wxwz_ref < vtype > wxwz()
	{
		return vtype4_wxwz_ref < vtype > (this);
	}
	vtype4_wxww_ref < vtype > wxww()
	{
		return vtype4_wxww_ref < vtype > (this);
	}
	vtype4_wyxx_ref < vtype > wyxx()
	{
		return vtype4_wyxx_ref < vtype > (this);
	}
	vtype4_wyxy_ref < vtype > wyxy()
	{
		return vtype4_wyxy_ref < vtype > (this);
	}
	vtype4_wyxz_ref < vtype > wyxz()
	{
		return vtype4_wyxz_ref < vtype > (this);
	}
	vtype4_wyxw_ref < vtype > wyxw()
	{
		return vtype4_wyxw_ref < vtype > (this);
	}
	vtype4_wyyx_ref < vtype > wyyx()
	{
		return vtype4_wyyx_ref < vtype > (this);
	}
	vtype4_wyyy_ref < vtype > wyyy()
	{
		return vtype4_wyyy_ref < vtype > (this);
	}
	vtype4_wyyz_ref < vtype > wyyz()
	{
		return vtype4_wyyz_ref < vtype > (this);
	}
	vtype4_wyyw_ref < vtype > wyyw()
	{
		return vtype4_wyyw_ref < vtype > (this);
	}
	vtype4_wyzx_ref < vtype > wyzx()
	{
		return vtype4_wyzx_ref < vtype > (this);
	}
	vtype4_wyzy_ref < vtype > wyzy()
	{
		return vtype4_wyzy_ref < vtype > (this);
	}
	vtype4_wyzz_ref < vtype > wyzz()
	{
		return vtype4_wyzz_ref < vtype > (this);
	}
	vtype4_wyzw_ref < vtype > wyzw()
	{
		return vtype4_wyzw_ref < vtype > (this);
	}
	vtype4_wywx_ref < vtype > wywx()
	{
		return vtype4_wywx_ref < vtype > (this);
	}
	vtype4_wywy_ref < vtype > wywy()
	{
		return vtype4_wywy_ref < vtype > (this);
	}
	vtype4_wywz_ref < vtype > wywz()
	{
		return vtype4_wywz_ref < vtype > (this);
	}
	vtype4_wyww_ref < vtype > wyww()
	{
		return vtype4_wyww_ref < vtype > (this);
	}
	vtype4_wzxx_ref < vtype > wzxx()
	{
		return vtype4_wzxx_ref < vtype > (this);
	}
	vtype4_wzxy_ref < vtype > wzxy()
	{
		return vtype4_wzxy_ref < vtype > (this);
	}
	vtype4_wzxz_ref < vtype > wzxz()
	{
		return vtype4_wzxz_ref < vtype > (this);
	}
	vtype4_wzxw_ref < vtype > wzxw()
	{
		return vtype4_wzxw_ref < vtype > (this);
	}
	vtype4_wzyx_ref < vtype > wzyx()
	{
		return vtype4_wzyx_ref < vtype > (this);
	}
	vtype4_wzyy_ref < vtype > wzyy()
	{
		return vtype4_wzyy_ref < vtype > (this);
	}
	vtype4_wzyz_ref < vtype > wzyz()
	{
		return vtype4_wzyz_ref < vtype > (this);
	}
	vtype4_wzyw_ref < vtype > wzyw()
	{
		return vtype4_wzyw_ref < vtype > (this);
	}
	vtype4_wzzx_ref < vtype > wzzx()
	{
		return vtype4_wzzx_ref < vtype > (this);
	}
	vtype4_wzzy_ref < vtype > wzzy()
	{
		return vtype4_wzzy_ref < vtype > (this);
	}
	vtype4_wzzz_ref < vtype > wzzz()
	{
		return vtype4_wzzz_ref < vtype > (this);
	}
	vtype4_wzzw_ref < vtype > wzzw()
	{
		return vtype4_wzzw_ref < vtype > (this);
	}
	vtype4_wzwx_ref < vtype > wzwx()
	{
		return vtype4_wzwx_ref < vtype > (this);
	}
	vtype4_wzwy_ref < vtype > wzwy()
	{
		return vtype4_wzwy_ref < vtype > (this);
	}
	vtype4_wzwz_ref < vtype > wzwz()
	{
		return vtype4_wzwz_ref < vtype > (this);
	}
	vtype4_wzww_ref < vtype > wzww()
	{
		return vtype4_wzww_ref < vtype > (this);
	}
	vtype4_wwxx_ref < vtype > wwxx()
	{
		return vtype4_wwxx_ref < vtype > (this);
	}
	vtype4_wwxy_ref < vtype > wwxy()
	{
		return vtype4_wwxy_ref < vtype > (this);
	}
	vtype4_wwxz_ref < vtype > wwxz()
	{
		return vtype4_wwxz_ref < vtype > (this);
	}
	vtype4_wwxw_ref < vtype > wwxw()
	{
		return vtype4_wwxw_ref < vtype > (this);
	}
	vtype4_wwyx_ref < vtype > wwyx()
	{
		return vtype4_wwyx_ref < vtype > (this);
	}
	vtype4_wwyy_ref < vtype > wwyy()
	{
		return vtype4_wwyy_ref < vtype > (this);
	}
	vtype4_wwyz_ref < vtype > wwyz()
	{
		return vtype4_wwyz_ref < vtype > (this);
	}
	vtype4_wwyw_ref < vtype > wwyw()
	{
		return vtype4_wwyw_ref < vtype > (this);
	}
	vtype4_wwzx_ref < vtype > wwzx()
	{
		return vtype4_wwzx_ref < vtype > (this);
	}
	vtype4_wwzy_ref < vtype > wwzy()
	{
		return vtype4_wwzy_ref < vtype > (this);
	}
	vtype4_wwzz_ref < vtype > wwzz()
	{
		return vtype4_wwzz_ref < vtype > (this);
	}
	vtype4_wwzw_ref < vtype > wwzw()
	{
		return vtype4_wwzw_ref < vtype > (this);
	}
	vtype4_wwwx_ref < vtype > wwwx()
	{
		return vtype4_wwwx_ref < vtype > (this);
	}
	vtype4_wwwy_ref < vtype > wwwy()
	{
		return vtype4_wwwy_ref < vtype > (this);
	}
	vtype4_wwwz_ref < vtype > wwwz()
	{
		return vtype4_wwwz_ref < vtype > (this);
	}
	vtype4_wwww_ref < vtype > wwww()
	{
		return vtype4_wwww_ref < vtype > (this);
	}
};

template < typename vtype > vtype2_xy_ref < vtype > &vtype2_xy_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->x = other.x;
	v->y = other.y;
	return *this;
}

template < typename vtype > vtype2_yx_ref < vtype > &vtype2_yx_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->y = other.x;
	v->x = other.y;
	return *this;
}

template < typename vtype > vtype3_xy_ref < vtype > &vtype3_xy_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->x = other.x;
	v->y = other.y;
	return *this;
}

template < typename vtype > vtype3_xz_ref < vtype > &vtype3_xz_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->x = other.x;
	v->z = other.y;
	return *this;
}

template < typename vtype > vtype3_yx_ref < vtype > &vtype3_yx_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->y = other.x;
	v->x = other.y;
	return *this;
}

template < typename vtype > vtype3_yz_ref < vtype > &vtype3_yz_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->y = other.x;
	v->z = other.y;
	return *this;
}

template < typename vtype > vtype3_zx_ref < vtype > &vtype3_zx_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->z = other.x;
	v->x = other.y;
	return *this;
}

template < typename vtype > vtype3_zy_ref < vtype > &vtype3_zy_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->z = other.x;
	v->y = other.y;
	return *this;
}

template < typename vtype > vtype3_xyz_ref < vtype > &vtype3_xyz_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->x = other.x;
	v->y = other.y;
	v->z = other.z;
	return *this;
}

template < typename vtype > vtype3_xzy_ref < vtype > &vtype3_xzy_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->x = other.x;
	v->z = other.y;
	v->y = other.z;
	return *this;
}

template < typename vtype > vtype3_yxz_ref < vtype > &vtype3_yxz_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->y = other.x;
	v->x = other.y;
	v->z = other.z;
	return *this;
}

template < typename vtype > vtype3_yzx_ref < vtype > &vtype3_yzx_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->y = other.x;
	v->z = other.y;
	v->x = other.z;
	return *this;
}

template < typename vtype > vtype3_zxy_ref < vtype > &vtype3_zxy_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->z = other.x;
	v->x = other.y;
	v->y = other.z;
	return *this;
}

template < typename vtype > vtype3_zyx_ref < vtype > &vtype3_zyx_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->z = other.x;
	v->y = other.y;
	v->x = other.z;
	return *this;
}

template < typename vtype > vtype4_xy_ref < vtype > &vtype4_xy_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->x = other.x;
	v->y = other.y;
	return *this;
}

template < typename vtype > vtype4_xz_ref < vtype > &vtype4_xz_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->x = other.x;
	v->z = other.y;
	return *this;
}

template < typename vtype > vtype4_xw_ref < vtype > &vtype4_xw_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->x = other.x;
	v->w = other.y;
	return *this;
}

template < typename vtype > vtype4_yx_ref < vtype > &vtype4_yx_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->y = other.x;
	v->x = other.y;
	return *this;
}

template < typename vtype > vtype4_yz_ref < vtype > &vtype4_yz_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->y = other.x;
	v->z = other.y;
	return *this;
}

template < typename vtype > vtype4_yw_ref < vtype > &vtype4_yw_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->y = other.x;
	v->w = other.y;
	return *this;
}

template < typename vtype > vtype4_zx_ref < vtype > &vtype4_zx_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->z = other.x;
	v->x = other.y;
	return *this;
}

template < typename vtype > vtype4_zy_ref < vtype > &vtype4_zy_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->z = other.x;
	v->y = other.y;
	return *this;
}

template < typename vtype > vtype4_zw_ref < vtype > &vtype4_zw_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->z = other.x;
	v->w = other.y;
	return *this;
}

template < typename vtype > vtype4_wx_ref < vtype > &vtype4_wx_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->w = other.x;
	v->x = other.y;
	return *this;
}

template < typename vtype > vtype4_wy_ref < vtype > &vtype4_wy_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->w = other.x;
	v->y = other.y;
	return *this;
}

template < typename vtype > vtype4_wz_ref < vtype > &vtype4_wz_ref < vtype >::operator=(const vtype2 < vtype > &other)
{
	v->w = other.x;
	v->z = other.y;
	return *this;
}

template < typename vtype > vtype4_xyz_ref < vtype > &vtype4_xyz_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->x = other.x;
	v->y = other.y;
	v->z = other.z;
	return *this;
}

template < typename vtype > vtype4_xyw_ref < vtype > &vtype4_xyw_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->x = other.x;
	v->y = other.y;
	v->w = other.z;
	return *this;
}

template < typename vtype > vtype4_xzy_ref < vtype > &vtype4_xzy_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->x = other.x;
	v->z = other.y;
	v->y = other.z;
	return *this;
}

template < typename vtype > vtype4_xzw_ref < vtype > &vtype4_xzw_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->x = other.x;
	v->z = other.y;
	v->w = other.z;
	return *this;
}

template < typename vtype > vtype4_xwy_ref < vtype > &vtype4_xwy_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->x = other.x;
	v->w = other.y;
	v->y = other.z;
	return *this;
}

template < typename vtype > vtype4_xwz_ref < vtype > &vtype4_xwz_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->x = other.x;
	v->w = other.y;
	v->z = other.z;
	return *this;
}

template < typename vtype > vtype4_yxz_ref < vtype > &vtype4_yxz_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->y = other.x;
	v->x = other.y;
	v->z = other.z;
	return *this;
}

template < typename vtype > vtype4_yxw_ref < vtype > &vtype4_yxw_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->y = other.x;
	v->x = other.y;
	v->w = other.z;
	return *this;
}

template < typename vtype > vtype4_yzx_ref < vtype > &vtype4_yzx_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->y = other.x;
	v->z = other.y;
	v->x = other.z;
	return *this;
}

template < typename vtype > vtype4_yzw_ref < vtype > &vtype4_yzw_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->y = other.x;
	v->z = other.y;
	v->w = other.z;
	return *this;
}

template < typename vtype > vtype4_ywx_ref < vtype > &vtype4_ywx_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->y = other.x;
	v->w = other.y;
	v->x = other.z;
	return *this;
}

template < typename vtype > vtype4_ywz_ref < vtype > &vtype4_ywz_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->y = other.x;
	v->w = other.y;
	v->z = other.z;
	return *this;
}

template < typename vtype > vtype4_zxy_ref < vtype > &vtype4_zxy_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->z = other.x;
	v->x = other.y;
	v->y = other.z;
	return *this;
}

template < typename vtype > vtype4_zxw_ref < vtype > &vtype4_zxw_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->z = other.x;
	v->x = other.y;
	v->w = other.z;
	return *this;
}

template < typename vtype > vtype4_zyx_ref < vtype > &vtype4_zyx_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->z = other.x;
	v->y = other.y;
	v->x = other.z;
	return *this;
}

template < typename vtype > vtype4_zyw_ref < vtype > &vtype4_zyw_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->z = other.x;
	v->y = other.y;
	v->w = other.z;
	return *this;
}

template < typename vtype > vtype4_zwx_ref < vtype > &vtype4_zwx_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->z = other.x;
	v->w = other.y;
	v->x = other.z;
	return *this;
}

template < typename vtype > vtype4_zwy_ref < vtype > &vtype4_zwy_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->z = other.x;
	v->w = other.y;
	v->y = other.z;
	return *this;
}

template < typename vtype > vtype4_wxy_ref < vtype > &vtype4_wxy_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->w = other.x;
	v->x = other.y;
	v->y = other.z;
	return *this;
}

template < typename vtype > vtype4_wxz_ref < vtype > &vtype4_wxz_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->w = other.x;
	v->x = other.y;
	v->z = other.z;
	return *this;
}

template < typename vtype > vtype4_wyx_ref < vtype > &vtype4_wyx_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->w = other.x;
	v->y = other.y;
	v->x = other.z;
	return *this;
}

template < typename vtype > vtype4_wyz_ref < vtype > &vtype4_wyz_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->w = other.x;
	v->y = other.y;
	v->z = other.z;
	return *this;
}

template < typename vtype > vtype4_wzx_ref < vtype > &vtype4_wzx_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->w = other.x;
	v->z = other.y;
	v->x = other.z;
	return *this;
}

template < typename vtype > vtype4_wzy_ref < vtype > &vtype4_wzy_ref < vtype >::operator=(const vtype3 < vtype > &other)
{
	v->w = other.x;
	v->z = other.y;
	v->y = other.z;
	return *this;
}

template < typename vtype > vtype4_xyzw_ref < vtype > &vtype4_xyzw_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->x = other.x;
	v->y = other.y;
	v->z = other.z;
	v->w = other.w;
	return *this;
}

template < typename vtype > vtype4_xywz_ref < vtype > &vtype4_xywz_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->x = other.x;
	v->y = other.y;
	v->w = other.z;
	v->z = other.w;
	return *this;
}

template < typename vtype > vtype4_xzyw_ref < vtype > &vtype4_xzyw_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->x = other.x;
	v->z = other.y;
	v->y = other.z;
	v->w = other.w;
	return *this;
}

template < typename vtype > vtype4_xzwy_ref < vtype > &vtype4_xzwy_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->x = other.x;
	v->z = other.y;
	v->w = other.z;
	v->y = other.w;
	return *this;
}

template < typename vtype > vtype4_xwyz_ref < vtype > &vtype4_xwyz_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->x = other.x;
	v->w = other.y;
	v->y = other.z;
	v->z = other.w;
	return *this;
}

template < typename vtype > vtype4_xwzy_ref < vtype > &vtype4_xwzy_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->x = other.x;
	v->w = other.y;
	v->z = other.z;
	v->y = other.w;
	return *this;
}

template < typename vtype > vtype4_yxzw_ref < vtype > &vtype4_yxzw_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->y = other.x;
	v->x = other.y;
	v->z = other.z;
	v->w = other.w;
	return *this;
}

template < typename vtype > vtype4_yxwz_ref < vtype > &vtype4_yxwz_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->y = other.x;
	v->x = other.y;
	v->w = other.z;
	v->z = other.w;
	return *this;
}

template < typename vtype > vtype4_yzxw_ref < vtype > &vtype4_yzxw_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->y = other.x;
	v->z = other.y;
	v->x = other.z;
	v->w = other.w;
	return *this;
}

template < typename vtype > vtype4_yzwx_ref < vtype > &vtype4_yzwx_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->y = other.x;
	v->z = other.y;
	v->w = other.z;
	v->x = other.w;
	return *this;
}

template < typename vtype > vtype4_ywxz_ref < vtype > &vtype4_ywxz_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->y = other.x;
	v->w = other.y;
	v->x = other.z;
	v->z = other.w;
	return *this;
}

template < typename vtype > vtype4_ywzx_ref < vtype > &vtype4_ywzx_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->y = other.x;
	v->w = other.y;
	v->z = other.z;
	v->x = other.w;
	return *this;
}

template < typename vtype > vtype4_zxyw_ref < vtype > &vtype4_zxyw_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->z = other.x;
	v->x = other.y;
	v->y = other.z;
	v->w = other.w;
	return *this;
}

template < typename vtype > vtype4_zxwy_ref < vtype > &vtype4_zxwy_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->z = other.x;
	v->x = other.y;
	v->w = other.z;
	v->y = other.w;
	return *this;
}

template < typename vtype > vtype4_zyxw_ref < vtype > &vtype4_zyxw_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->z = other.x;
	v->y = other.y;
	v->x = other.z;
	v->w = other.w;
	return *this;
}

template < typename vtype > vtype4_zywx_ref < vtype > &vtype4_zywx_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->z = other.x;
	v->y = other.y;
	v->w = other.z;
	v->x = other.w;
	return *this;
}

template < typename vtype > vtype4_zwxy_ref < vtype > &vtype4_zwxy_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->z = other.x;
	v->w = other.y;
	v->x = other.z;
	v->y = other.w;
	return *this;
}

template < typename vtype > vtype4_zwyx_ref < vtype > &vtype4_zwyx_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->z = other.x;
	v->w = other.y;
	v->y = other.z;
	v->x = other.w;
	return *this;
}

template < typename vtype > vtype4_wxyz_ref < vtype > &vtype4_wxyz_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->w = other.x;
	v->x = other.y;
	v->y = other.z;
	v->z = other.w;
	return *this;
}

template < typename vtype > vtype4_wxzy_ref < vtype > &vtype4_wxzy_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->w = other.x;
	v->x = other.y;
	v->z = other.z;
	v->y = other.w;
	return *this;
}

template < typename vtype > vtype4_wyxz_ref < vtype > &vtype4_wyxz_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->w = other.x;
	v->y = other.y;
	v->x = other.z;
	v->z = other.w;
	return *this;
}

template < typename vtype > vtype4_wyzx_ref < vtype > &vtype4_wyzx_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->w = other.x;
	v->y = other.y;
	v->z = other.z;
	v->x = other.w;
	return *this;
}

template < typename vtype > vtype4_wzxy_ref < vtype > &vtype4_wzxy_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->w = other.x;
	v->z = other.y;
	v->x = other.z;
	v->y = other.w;
	return *this;
}

template < typename vtype > vtype4_wzyx_ref < vtype > &vtype4_wzyx_ref < vtype >::operator=(const vtype4 < vtype > &other)
{
	v->w = other.x;
	v->z = other.y;
	v->y = other.z;
	v->x = other.w;
	return *this;
}

template < typename vtype > vtype2 < vtype >::vtype2(const vtype2_xx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype2_xy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype2_yx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype2_yy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_xx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_xy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_xz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_yx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_yy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_yz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_zx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_zy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype3_zz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_xx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_xy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_xz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_xw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_yx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_yy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_yz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_yw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_zx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_zy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_zz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_zw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_wx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_wy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_wz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
}
template < typename vtype > vtype2 < vtype >::vtype2(const vtype4_ww_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype2_xxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype2_xxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype2_xyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype2_xyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype2_yxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype2_yxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype2_yyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype2_yyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_xzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_yzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype3_zzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xxw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xyw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xzw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xwx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xwy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xwz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_xww_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yxw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yyw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yzw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_ywx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_ywy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_ywz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_yww_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zxw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zyw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zzw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zwx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zwy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zwz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_zww_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wxx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wxy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wxz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wxw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wyx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wyy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wyz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wyw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wzx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wzy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wzz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wzw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->w;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wwx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->x;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wwy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->y;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_wwz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->z;
}
template < typename vtype > vtype3 < vtype >::vtype3(const vtype4_www_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_xxxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_xxxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_xxyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_xxyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_xyxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_xyxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_xyyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_xyyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_yxxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_yxxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_yxyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_yxyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_yyxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_yyxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_yyyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype2_yyyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xxzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xyzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_xzzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yxzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yyzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_yzzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zxzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zyzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype3_zzzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxxw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxyw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxzw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxwx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxwy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxwz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xxww_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->x;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyxw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyyw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyzw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xywx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xywy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xywz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xyww_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->y;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzxw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzyw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzzw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzwx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzwy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzwz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xzww_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->z;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwxx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwxy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwxz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwxw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwyx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwyy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwyz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwyw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwzx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwzy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwzz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwzw_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwwx_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwwy_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwwz_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_xwww_ref < vtype > &v)
{
	x = v.v->x;
	y = v.v->w;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxxw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxyw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxzw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxwx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxwy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxwz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yxww_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->x;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyxw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyyw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyzw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yywx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yywy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yywz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yyww_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->y;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzxw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzyw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzzw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzwx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzwy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzwz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_yzww_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->z;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywxx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywxy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywxz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywxw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywyx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywyy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywyz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywyw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywzx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywzy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywzz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywzw_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywwx_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywwy_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywwz_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_ywww_ref < vtype > &v)
{
	x = v.v->y;
	y = v.v->w;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxxw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxyw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxzw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxwx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxwy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxwz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zxww_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->x;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyxw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyyw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyzw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zywx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zywy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zywz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zyww_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->y;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzxw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzyw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzzw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzwx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzwy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzwz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zzww_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->z;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwxx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwxy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwxz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwxw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwyx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwyy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwyz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwyw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwzx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwzy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwzz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwzw_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwwx_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwwy_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwwz_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_zwww_ref < vtype > &v)
{
	x = v.v->z;
	y = v.v->w;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxxx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxxy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxxz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxxw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxyx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxyy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxyz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxyw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxzx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxzy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxzz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxzw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxwx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxwy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxwz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wxww_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->x;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyxx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyxy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyxz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyxw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyyx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyyy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyyz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyyw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyzx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyzy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyzz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyzw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wywx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wywy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wywz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wyww_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->y;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzxx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzxy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzxz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzxw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzyx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzyy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzyz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzyw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzzx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzzy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzzz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzzw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzwx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzwy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzwz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wzww_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->z;
	z = v.v->w;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwxx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->x;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwxy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->x;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwxz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->x;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwxw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->x;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwyx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->y;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwyy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->y;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwyz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->y;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwyw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->y;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwzx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->z;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwzy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->z;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwzz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->z;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwzw_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->z;
	w = v.v->w;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwwx_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->w;
	w = v.v->x;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwwy_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->w;
	w = v.v->y;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwwz_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->w;
	w = v.v->z;
}
template < typename vtype > vtype4 < vtype >::vtype4(const vtype4_wwww_ref < vtype > &v)
{
	x = v.v->w;
	y = v.v->w;
	z = v.v->w;
	w = v.v->w;
}
typedef vtype2 < float >float2;
typedef vtype3 < float >float3;
typedef vtype4 < float >float4;
typedef vtype2 < double >double2;
typedef vtype3 < double >double3;
typedef vtype4 < double >double4;
typedef vtype2 < int >int2;
typedef vtype3 < int >int3;
typedef vtype4 < int >int4;
typedef vtype2 < uint > uint2;
typedef vtype3 < uint > uint3;
typedef vtype4 < uint > uint4;
typedef vtype2 < short >short2;
typedef vtype3 < short >short3;
typedef vtype4 < short >short4;
typedef vtype2 < ushort > ushort2;
typedef vtype3 < ushort > ushort3;
typedef vtype4 < ushort > ushort4;
typedef vtype2 < long >long2;
typedef vtype3 < long >long3;
typedef vtype4 < long >long4;
typedef vtype2 < ulong > ulong2;
typedef vtype3 < ulong > ulong3;
typedef vtype4 < ulong > ulong4;
static inline float2 operator+(float2 p, float2 q)
{
	return float2(p.x + q.x, p.y + q.y);
}
static inline float3 operator+(float3 p, float3 q)
{
	return float3(p.x + q.x, p.y + q.y, p.z + q.z);
}
static inline float4 operator+(float4 p, float4 q)
{
	return float4(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}
static inline float2 operator+(float2 p, float q)
{
	return float2(p.x + q, p.y + q);
}
static inline float3 operator+(float3 p, float q)
{
	return float3(p.x + q, p.y + q, p.z + q);
}
static inline float4 operator+(float4 p, float q)
{
	return float4(p.x + q, p.y + q, p.z + q, p.w + q);
}
static inline float2 operator+(float p, float2 q)
{
	return float2(p + q.x, p + q.y);
}
static inline float3 operator+(float p, float3 q)
{
	return float3(p + q.x, p + q.y, p + q.z);
}
static inline float4 operator+(float p, float4 q)
{
	return float4(p + q.x, p + q.y, p + q.z, p + q.w);
}
static inline double2 operator+(double2 p, double2 q)
{
	return double2(p.x + q.x, p.y + q.y);
}
static inline double3 operator+(double3 p, double3 q)
{
	return double3(p.x + q.x, p.y + q.y, p.z + q.z);
}
static inline double4 operator+(double4 p, double4 q)
{
	return double4(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}
static inline double2 operator+(double2 p, double q)
{
	return double2(p.x + q, p.y + q);
}
static inline double3 operator+(double3 p, double q)
{
	return double3(p.x + q, p.y + q, p.z + q);
}
static inline double4 operator+(double4 p, double q)
{
	return double4(p.x + q, p.y + q, p.z + q, p.w + q);
}
static inline double2 operator+(double p, double2 q)
{
	return double2(p + q.x, p + q.y);
}
static inline double3 operator+(double p, double3 q)
{
	return double3(p + q.x, p + q.y, p + q.z);
}
static inline double4 operator+(double p, double4 q)
{
	return double4(p + q.x, p + q.y, p + q.z, p + q.w);
}
static inline int2 operator+(int2 p, int2 q)
{
	return int2(p.x + q.x, p.y + q.y);
}
static inline int3 operator+(int3 p, int3 q)
{
	return int3(p.x + q.x, p.y + q.y, p.z + q.z);
}
static inline int4 operator+(int4 p, int4 q)
{
	return int4(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}
static inline int2 operator+(int2 p, int q)
{
	return int2(p.x + q, p.y + q);
}
static inline int3 operator+(int3 p, int q)
{
	return int3(p.x + q, p.y + q, p.z + q);
}
static inline int4 operator+(int4 p, int q)
{
	return int4(p.x + q, p.y + q, p.z + q, p.w + q);
}
static inline int2 operator+(int p, int2 q)
{
	return int2(p + q.x, p + q.y);
}
static inline int3 operator+(int p, int3 q)
{
	return int3(p + q.x, p + q.y, p + q.z);
}
static inline int4 operator+(int p, int4 q)
{
	return int4(p + q.x, p + q.y, p + q.z, p + q.w);
}
static inline uint2 operator+(uint2 p, uint2 q)
{
	return uint2(p.x + q.x, p.y + q.y);
}
static inline uint3 operator+(uint3 p, uint3 q)
{
	return uint3(p.x + q.x, p.y + q.y, p.z + q.z);
}
static inline uint4 operator+(uint4 p, uint4 q)
{
	return uint4(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}
static inline uint2 operator+(uint2 p, uint q)
{
	return uint2(p.x + q, p.y + q);
}
static inline uint3 operator+(uint3 p, uint q)
{
	return uint3(p.x + q, p.y + q, p.z + q);
}
static inline uint4 operator+(uint4 p, uint q)
{
	return uint4(p.x + q, p.y + q, p.z + q, p.w + q);
}
static inline uint2 operator+(uint p, uint2 q)
{
	return uint2(p + q.x, p + q.y);
}
static inline uint3 operator+(uint p, uint3 q)
{
	return uint3(p + q.x, p + q.y, p + q.z);
}
static inline uint4 operator+(uint p, uint4 q)
{
	return uint4(p + q.x, p + q.y, p + q.z, p + q.w);
}
static inline short2 operator+(short2 p, short2 q)
{
	return short2(p.x + q.x, p.y + q.y);
}
static inline short3 operator+(short3 p, short3 q)
{
	return short3(p.x + q.x, p.y + q.y, p.z + q.z);
}
static inline short4 operator+(short4 p, short4 q)
{
	return short4(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}
static inline short2 operator+(short2 p, short q)
{
	return short2(p.x + q, p.y + q);
}
static inline short3 operator+(short3 p, short q)
{
	return short3(p.x + q, p.y + q, p.z + q);
}
static inline short4 operator+(short4 p, short q)
{
	return short4(p.x + q, p.y + q, p.z + q, p.w + q);
}
static inline short2 operator+(short p, short2 q)
{
	return short2(p + q.x, p + q.y);
}
static inline short3 operator+(short p, short3 q)
{
	return short3(p + q.x, p + q.y, p + q.z);
}
static inline short4 operator+(short p, short4 q)
{
	return short4(p + q.x, p + q.y, p + q.z, p + q.w);
}
static inline ushort2 operator+(ushort2 p, ushort2 q)
{
	return ushort2(p.x + q.x, p.y + q.y);
}
static inline ushort3 operator+(ushort3 p, ushort3 q)
{
	return ushort3(p.x + q.x, p.y + q.y, p.z + q.z);
}
static inline ushort4 operator+(ushort4 p, ushort4 q)
{
	return ushort4(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}
static inline ushort2 operator+(ushort2 p, ushort q)
{
	return ushort2(p.x + q, p.y + q);
}
static inline ushort3 operator+(ushort3 p, ushort q)
{
	return ushort3(p.x + q, p.y + q, p.z + q);
}
static inline ushort4 operator+(ushort4 p, ushort q)
{
	return ushort4(p.x + q, p.y + q, p.z + q, p.w + q);
}
static inline ushort2 operator+(ushort p, ushort2 q)
{
	return ushort2(p + q.x, p + q.y);
}
static inline ushort3 operator+(ushort p, ushort3 q)
{
	return ushort3(p + q.x, p + q.y, p + q.z);
}
static inline ushort4 operator+(ushort p, ushort4 q)
{
	return ushort4(p + q.x, p + q.y, p + q.z, p + q.w);
}
static inline long2 operator+(long2 p, long2 q)
{
	return long2(p.x + q.x, p.y + q.y);
}
static inline long3 operator+(long3 p, long3 q)
{
	return long3(p.x + q.x, p.y + q.y, p.z + q.z);
}
static inline long4 operator+(long4 p, long4 q)
{
	return long4(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}
static inline long2 operator+(long2 p, long q)
{
	return long2(p.x + q, p.y + q);
}
static inline long3 operator+(long3 p, long q)
{
	return long3(p.x + q, p.y + q, p.z + q);
}
static inline long4 operator+(long4 p, long q)
{
	return long4(p.x + q, p.y + q, p.z + q, p.w + q);
}
static inline long2 operator+(long p, long2 q)
{
	return long2(p + q.x, p + q.y);
}
static inline long3 operator+(long p, long3 q)
{
	return long3(p + q.x, p + q.y, p + q.z);
}
static inline long4 operator+(long p, long4 q)
{
	return long4(p + q.x, p + q.y, p + q.z, p + q.w);
}
static inline ulong2 operator+(ulong2 p, ulong2 q)
{
	return ulong2(p.x + q.x, p.y + q.y);
}
static inline ulong3 operator+(ulong3 p, ulong3 q)
{
	return ulong3(p.x + q.x, p.y + q.y, p.z + q.z);
}
static inline ulong4 operator+(ulong4 p, ulong4 q)
{
	return ulong4(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}
static inline ulong2 operator+(ulong2 p, ulong q)
{
	return ulong2(p.x + q, p.y + q);
}
static inline ulong3 operator+(ulong3 p, ulong q)
{
	return ulong3(p.x + q, p.y + q, p.z + q);
}
static inline ulong4 operator+(ulong4 p, ulong q)
{
	return ulong4(p.x + q, p.y + q, p.z + q, p.w + q);
}
static inline ulong2 operator+(ulong p, ulong2 q)
{
	return ulong2(p + q.x, p + q.y);
}
static inline ulong3 operator+(ulong p, ulong3 q)
{
	return ulong3(p + q.x, p + q.y, p + q.z);
}
static inline ulong4 operator+(ulong p, ulong4 q)
{
	return ulong4(p + q.x, p + q.y, p + q.z, p + q.w);
}
static inline float2 operator-(float2 p, float2 q)
{
	return float2(p.x - q.x, p.y - q.y);
}
static inline float3 operator-(float3 p, float3 q)
{
	return float3(p.x - q.x, p.y - q.y, p.z - q.z);
}
static inline float4 operator-(float4 p, float4 q)
{
	return float4(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}
static inline float2 operator-(float2 p, float q)
{
	return float2(p.x - q, p.y - q);
}
static inline float3 operator-(float3 p, float q)
{
	return float3(p.x - q, p.y - q, p.z - q);
}
static inline float4 operator-(float4 p, float q)
{
	return float4(p.x - q, p.y - q, p.z - q, p.w - q);
}
static inline float2 operator-(float p, float2 q)
{
	return float2(p - q.x, p - q.y);
}
static inline float3 operator-(float p, float3 q)
{
	return float3(p - q.x, p - q.y, p - q.z);
}
static inline float4 operator-(float p, float4 q)
{
	return float4(p - q.x, p - q.y, p - q.z, p - q.w);
}
static inline double2 operator-(double2 p, double2 q)
{
	return double2(p.x - q.x, p.y - q.y);
}
static inline double3 operator-(double3 p, double3 q)
{
	return double3(p.x - q.x, p.y - q.y, p.z - q.z);
}
static inline double4 operator-(double4 p, double4 q)
{
	return double4(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}
static inline double2 operator-(double2 p, double q)
{
	return double2(p.x - q, p.y - q);
}
static inline double3 operator-(double3 p, double q)
{
	return double3(p.x - q, p.y - q, p.z - q);
}
static inline double4 operator-(double4 p, double q)
{
	return double4(p.x - q, p.y - q, p.z - q, p.w - q);
}
static inline double2 operator-(double p, double2 q)
{
	return double2(p - q.x, p - q.y);
}
static inline double3 operator-(double p, double3 q)
{
	return double3(p - q.x, p - q.y, p - q.z);
}
static inline double4 operator-(double p, double4 q)
{
	return double4(p - q.x, p - q.y, p - q.z, p - q.w);
}
static inline int2 operator-(int2 p, int2 q)
{
	return int2(p.x - q.x, p.y - q.y);
}
static inline int3 operator-(int3 p, int3 q)
{
	return int3(p.x - q.x, p.y - q.y, p.z - q.z);
}
static inline int4 operator-(int4 p, int4 q)
{
	return int4(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}
static inline int2 operator-(int2 p, int q)
{
	return int2(p.x - q, p.y - q);
}
static inline int3 operator-(int3 p, int q)
{
	return int3(p.x - q, p.y - q, p.z - q);
}
static inline int4 operator-(int4 p, int q)
{
	return int4(p.x - q, p.y - q, p.z - q, p.w - q);
}
static inline int2 operator-(int p, int2 q)
{
	return int2(p - q.x, p - q.y);
}
static inline int3 operator-(int p, int3 q)
{
	return int3(p - q.x, p - q.y, p - q.z);
}
static inline int4 operator-(int p, int4 q)
{
	return int4(p - q.x, p - q.y, p - q.z, p - q.w);
}
static inline uint2 operator-(uint2 p, uint2 q)
{
	return uint2(p.x - q.x, p.y - q.y);
}
static inline uint3 operator-(uint3 p, uint3 q)
{
	return uint3(p.x - q.x, p.y - q.y, p.z - q.z);
}
static inline uint4 operator-(uint4 p, uint4 q)
{
	return uint4(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}
static inline uint2 operator-(uint2 p, uint q)
{
	return uint2(p.x - q, p.y - q);
}
static inline uint3 operator-(uint3 p, uint q)
{
	return uint3(p.x - q, p.y - q, p.z - q);
}
static inline uint4 operator-(uint4 p, uint q)
{
	return uint4(p.x - q, p.y - q, p.z - q, p.w - q);
}
static inline uint2 operator-(uint p, uint2 q)
{
	return uint2(p - q.x, p - q.y);
}
static inline uint3 operator-(uint p, uint3 q)
{
	return uint3(p - q.x, p - q.y, p - q.z);
}
static inline uint4 operator-(uint p, uint4 q)
{
	return uint4(p - q.x, p - q.y, p - q.z, p - q.w);
}
static inline short2 operator-(short2 p, short2 q)
{
	return short2(p.x - q.x, p.y - q.y);
}
static inline short3 operator-(short3 p, short3 q)
{
	return short3(p.x - q.x, p.y - q.y, p.z - q.z);
}
static inline short4 operator-(short4 p, short4 q)
{
	return short4(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}
static inline short2 operator-(short2 p, short q)
{
	return short2(p.x - q, p.y - q);
}
static inline short3 operator-(short3 p, short q)
{
	return short3(p.x - q, p.y - q, p.z - q);
}
static inline short4 operator-(short4 p, short q)
{
	return short4(p.x - q, p.y - q, p.z - q, p.w - q);
}
static inline short2 operator-(short p, short2 q)
{
	return short2(p - q.x, p - q.y);
}
static inline short3 operator-(short p, short3 q)
{
	return short3(p - q.x, p - q.y, p - q.z);
}
static inline short4 operator-(short p, short4 q)
{
	return short4(p - q.x, p - q.y, p - q.z, p - q.w);
}
static inline ushort2 operator-(ushort2 p, ushort2 q)
{
	return ushort2(p.x - q.x, p.y - q.y);
}
static inline ushort3 operator-(ushort3 p, ushort3 q)
{
	return ushort3(p.x - q.x, p.y - q.y, p.z - q.z);
}
static inline ushort4 operator-(ushort4 p, ushort4 q)
{
	return ushort4(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}
static inline ushort2 operator-(ushort2 p, ushort q)
{
	return ushort2(p.x - q, p.y - q);
}
static inline ushort3 operator-(ushort3 p, ushort q)
{
	return ushort3(p.x - q, p.y - q, p.z - q);
}
static inline ushort4 operator-(ushort4 p, ushort q)
{
	return ushort4(p.x - q, p.y - q, p.z - q, p.w - q);
}
static inline ushort2 operator-(ushort p, ushort2 q)
{
	return ushort2(p - q.x, p - q.y);
}
static inline ushort3 operator-(ushort p, ushort3 q)
{
	return ushort3(p - q.x, p - q.y, p - q.z);
}
static inline ushort4 operator-(ushort p, ushort4 q)
{
	return ushort4(p - q.x, p - q.y, p - q.z, p - q.w);
}
static inline long2 operator-(long2 p, long2 q)
{
	return long2(p.x - q.x, p.y - q.y);
}
static inline long3 operator-(long3 p, long3 q)
{
	return long3(p.x - q.x, p.y - q.y, p.z - q.z);
}
static inline long4 operator-(long4 p, long4 q)
{
	return long4(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}
static inline long2 operator-(long2 p, long q)
{
	return long2(p.x - q, p.y - q);
}
static inline long3 operator-(long3 p, long q)
{
	return long3(p.x - q, p.y - q, p.z - q);
}
static inline long4 operator-(long4 p, long q)
{
	return long4(p.x - q, p.y - q, p.z - q, p.w - q);
}
static inline long2 operator-(long p, long2 q)
{
	return long2(p - q.x, p - q.y);
}
static inline long3 operator-(long p, long3 q)
{
	return long3(p - q.x, p - q.y, p - q.z);
}
static inline long4 operator-(long p, long4 q)
{
	return long4(p - q.x, p - q.y, p - q.z, p - q.w);
}
static inline ulong2 operator-(ulong2 p, ulong2 q)
{
	return ulong2(p.x - q.x, p.y - q.y);
}
static inline ulong3 operator-(ulong3 p, ulong3 q)
{
	return ulong3(p.x - q.x, p.y - q.y, p.z - q.z);
}
static inline ulong4 operator-(ulong4 p, ulong4 q)
{
	return ulong4(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}
static inline ulong2 operator-(ulong2 p, ulong q)
{
	return ulong2(p.x - q, p.y - q);
}
static inline ulong3 operator-(ulong3 p, ulong q)
{
	return ulong3(p.x - q, p.y - q, p.z - q);
}
static inline ulong4 operator-(ulong4 p, ulong q)
{
	return ulong4(p.x - q, p.y - q, p.z - q, p.w - q);
}
static inline ulong2 operator-(ulong p, ulong2 q)
{
	return ulong2(p - q.x, p - q.y);
}
static inline ulong3 operator-(ulong p, ulong3 q)
{
	return ulong3(p - q.x, p - q.y, p - q.z);
}
static inline ulong4 operator-(ulong p, ulong4 q)
{
	return ulong4(p - q.x, p - q.y, p - q.z, p - q.w);
}
static inline float2 operator*(float2 p, float2 q)
{
	return float2(p.x * q.x, p.y * q.y);
}
static inline float3 operator*(float3 p, float3 q)
{
	return float3(p.x * q.x, p.y * q.y, p.z * q.z);
}
static inline float4 operator*(float4 p, float4 q)
{
	return float4(p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w);
}
static inline float2 operator*(float2 p, float q)
{
	return float2(p.x * q, p.y * q);
}
static inline float3 operator*(float3 p, float q)
{
	return float3(p.x * q, p.y * q, p.z * q);
}
static inline float4 operator*(float4 p, float q)
{
	return float4(p.x * q, p.y * q, p.z * q, p.w * q);
}
static inline float2 operator*(float p, float2 q)
{
	return float2(p * q.x, p * q.y);
}
static inline float3 operator*(float p, float3 q)
{
	return float3(p * q.x, p * q.y, p * q.z);
}
static inline float4 operator*(float p, float4 q)
{
	return float4(p * q.x, p * q.y, p * q.z, p * q.w);
}
static inline double2 operator*(double2 p, double2 q)
{
	return double2(p.x * q.x, p.y * q.y);
}
static inline double3 operator*(double3 p, double3 q)
{
	return double3(p.x * q.x, p.y * q.y, p.z * q.z);
}
static inline double4 operator*(double4 p, double4 q)
{
	return double4(p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w);
}
static inline double2 operator*(double2 p, double q)
{
	return double2(p.x * q, p.y * q);
}
static inline double3 operator*(double3 p, double q)
{
	return double3(p.x * q, p.y * q, p.z * q);
}
static inline double4 operator*(double4 p, double q)
{
	return double4(p.x * q, p.y * q, p.z * q, p.w * q);
}
static inline double2 operator*(double p, double2 q)
{
	return double2(p * q.x, p * q.y);
}
static inline double3 operator*(double p, double3 q)
{
	return double3(p * q.x, p * q.y, p * q.z);
}
static inline double4 operator*(double p, double4 q)
{
	return double4(p * q.x, p * q.y, p * q.z, p * q.w);
}
static inline int2 operator*(int2 p, int2 q)
{
	return int2(p.x * q.x, p.y * q.y);
}
static inline int3 operator*(int3 p, int3 q)
{
	return int3(p.x * q.x, p.y * q.y, p.z * q.z);
}
static inline int4 operator*(int4 p, int4 q)
{
	return int4(p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w);
}
static inline int2 operator*(int2 p, int q)
{
	return int2(p.x * q, p.y * q);
}
static inline int3 operator*(int3 p, int q)
{
	return int3(p.x * q, p.y * q, p.z * q);
}
static inline int4 operator*(int4 p, int q)
{
	return int4(p.x * q, p.y * q, p.z * q, p.w * q);
}
static inline int2 operator*(int p, int2 q)
{
	return int2(p * q.x, p * q.y);
}
static inline int3 operator*(int p, int3 q)
{
	return int3(p * q.x, p * q.y, p * q.z);
}
static inline int4 operator*(int p, int4 q)
{
	return int4(p * q.x, p * q.y, p * q.z, p * q.w);
}
static inline uint2 operator*(uint2 p, uint2 q)
{
	return uint2(p.x * q.x, p.y * q.y);
}
static inline uint3 operator*(uint3 p, uint3 q)
{
	return uint3(p.x * q.x, p.y * q.y, p.z * q.z);
}
static inline uint4 operator*(uint4 p, uint4 q)
{
	return uint4(p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w);
}
static inline uint2 operator*(uint2 p, uint q)
{
	return uint2(p.x * q, p.y * q);
}
static inline uint3 operator*(uint3 p, uint q)
{
	return uint3(p.x * q, p.y * q, p.z * q);
}
static inline uint4 operator*(uint4 p, uint q)
{
	return uint4(p.x * q, p.y * q, p.z * q, p.w * q);
}
static inline uint2 operator*(uint p, uint2 q)
{
	return uint2(p * q.x, p * q.y);
}
static inline uint3 operator*(uint p, uint3 q)
{
	return uint3(p * q.x, p * q.y, p * q.z);
}
static inline uint4 operator*(uint p, uint4 q)
{
	return uint4(p * q.x, p * q.y, p * q.z, p * q.w);
}
static inline short2 operator*(short2 p, short2 q)
{
	return short2(p.x * q.x, p.y * q.y);
}
static inline short3 operator*(short3 p, short3 q)
{
	return short3(p.x * q.x, p.y * q.y, p.z * q.z);
}
static inline short4 operator*(short4 p, short4 q)
{
	return short4(p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w);
}
static inline short2 operator*(short2 p, short q)
{
	return short2(p.x * q, p.y * q);
}
static inline short3 operator*(short3 p, short q)
{
	return short3(p.x * q, p.y * q, p.z * q);
}
static inline short4 operator*(short4 p, short q)
{
	return short4(p.x * q, p.y * q, p.z * q, p.w * q);
}
static inline short2 operator*(short p, short2 q)
{
	return short2(p * q.x, p * q.y);
}
static inline short3 operator*(short p, short3 q)
{
	return short3(p * q.x, p * q.y, p * q.z);
}
static inline short4 operator*(short p, short4 q)
{
	return short4(p * q.x, p * q.y, p * q.z, p * q.w);
}
static inline ushort2 operator*(ushort2 p, ushort2 q)
{
	return ushort2(p.x * q.x, p.y * q.y);
}
static inline ushort3 operator*(ushort3 p, ushort3 q)
{
	return ushort3(p.x * q.x, p.y * q.y, p.z * q.z);
}
static inline ushort4 operator*(ushort4 p, ushort4 q)
{
	return ushort4(p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w);
}
static inline ushort2 operator*(ushort2 p, ushort q)
{
	return ushort2(p.x * q, p.y * q);
}
static inline ushort3 operator*(ushort3 p, ushort q)
{
	return ushort3(p.x * q, p.y * q, p.z * q);
}
static inline ushort4 operator*(ushort4 p, ushort q)
{
	return ushort4(p.x * q, p.y * q, p.z * q, p.w * q);
}
static inline ushort2 operator*(ushort p, ushort2 q)
{
	return ushort2(p * q.x, p * q.y);
}
static inline ushort3 operator*(ushort p, ushort3 q)
{
	return ushort3(p * q.x, p * q.y, p * q.z);
}
static inline ushort4 operator*(ushort p, ushort4 q)
{
	return ushort4(p * q.x, p * q.y, p * q.z, p * q.w);
}
static inline long2 operator*(long2 p, long2 q)
{
	return long2(p.x * q.x, p.y * q.y);
}
static inline long3 operator*(long3 p, long3 q)
{
	return long3(p.x * q.x, p.y * q.y, p.z * q.z);
}
static inline long4 operator*(long4 p, long4 q)
{
	return long4(p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w);
}
static inline long2 operator*(long2 p, long q)
{
	return long2(p.x * q, p.y * q);
}
static inline long3 operator*(long3 p, long q)
{
	return long3(p.x * q, p.y * q, p.z * q);
}
static inline long4 operator*(long4 p, long q)
{
	return long4(p.x * q, p.y * q, p.z * q, p.w * q);
}
static inline long2 operator*(long p, long2 q)
{
	return long2(p * q.x, p * q.y);
}
static inline long3 operator*(long p, long3 q)
{
	return long3(p * q.x, p * q.y, p * q.z);
}
static inline long4 operator*(long p, long4 q)
{
	return long4(p * q.x, p * q.y, p * q.z, p * q.w);
}
static inline ulong2 operator*(ulong2 p, ulong2 q)
{
	return ulong2(p.x * q.x, p.y * q.y);
}
static inline ulong3 operator*(ulong3 p, ulong3 q)
{
	return ulong3(p.x * q.x, p.y * q.y, p.z * q.z);
}
static inline ulong4 operator*(ulong4 p, ulong4 q)
{
	return ulong4(p.x * q.x, p.y * q.y, p.z * q.z, p.w * q.w);
}
static inline ulong2 operator*(ulong2 p, ulong q)
{
	return ulong2(p.x * q, p.y * q);
}
static inline ulong3 operator*(ulong3 p, ulong q)
{
	return ulong3(p.x * q, p.y * q, p.z * q);
}
static inline ulong4 operator*(ulong4 p, ulong q)
{
	return ulong4(p.x * q, p.y * q, p.z * q, p.w * q);
}
static inline ulong2 operator*(ulong p, ulong2 q)
{
	return ulong2(p * q.x, p * q.y);
}
static inline ulong3 operator*(ulong p, ulong3 q)
{
	return ulong3(p * q.x, p * q.y, p * q.z);
}
static inline ulong4 operator*(ulong p, ulong4 q)
{
	return ulong4(p * q.x, p * q.y, p * q.z, p * q.w);
}
static inline float2 operator/(float2 p, float2 q)
{
	return float2(p.x / q.x, p.y / q.y);
}
static inline float3 operator/(float3 p, float3 q)
{
	return float3(p.x / q.x, p.y / q.y, p.z / q.z);
}
static inline float4 operator/(float4 p, float4 q)
{
	return float4(p.x / q.x, p.y / q.y, p.z / q.z, p.w / q.w);
}
static inline float2 operator/(float2 p, float q)
{
	return float2(p.x / q, p.y / q);
}
static inline float3 operator/(float3 p, float q)
{
	return float3(p.x / q, p.y / q, p.z / q);
}
static inline float4 operator/(float4 p, float q)
{
	return float4(p.x / q, p.y / q, p.z / q, p.w / q);
}
static inline float2 operator/(float p, float2 q)
{
	return float2(p / q.x, p / q.y);
}
static inline float3 operator/(float p, float3 q)
{
	return float3(p / q.x, p / q.y, p / q.z);
}
static inline float4 operator/(float p, float4 q)
{
	return float4(p / q.x, p / q.y, p / q.z, p / q.w);
}
static inline double2 operator/(double2 p, double2 q)
{
	return double2(p.x / q.x, p.y / q.y);
}
static inline double3 operator/(double3 p, double3 q)
{
	return double3(p.x / q.x, p.y / q.y, p.z / q.z);
}
static inline double4 operator/(double4 p, double4 q)
{
	return double4(p.x / q.x, p.y / q.y, p.z / q.z, p.w / q.w);
}
static inline double2 operator/(double2 p, double q)
{
	return double2(p.x / q, p.y / q);
}
static inline double3 operator/(double3 p, double q)
{
	return double3(p.x / q, p.y / q, p.z / q);
}
static inline double4 operator/(double4 p, double q)
{
	return double4(p.x / q, p.y / q, p.z / q, p.w / q);
}
static inline double2 operator/(double p, double2 q)
{
	return double2(p / q.x, p / q.y);
}
static inline double3 operator/(double p, double3 q)
{
	return double3(p / q.x, p / q.y, p / q.z);
}
static inline double4 operator/(double p, double4 q)
{
	return double4(p / q.x, p / q.y, p / q.z, p / q.w);
}
static inline int2 operator/(int2 p, int2 q)
{
	return int2(p.x / q.x, p.y / q.y);
}
static inline int3 operator/(int3 p, int3 q)
{
	return int3(p.x / q.x, p.y / q.y, p.z / q.z);
}
static inline int4 operator/(int4 p, int4 q)
{
	return int4(p.x / q.x, p.y / q.y, p.z / q.z, p.w / q.w);
}
static inline int2 operator/(int2 p, int q)
{
	return int2(p.x / q, p.y / q);
}
static inline int3 operator/(int3 p, int q)
{
	return int3(p.x / q, p.y / q, p.z / q);
}
static inline int4 operator/(int4 p, int q)
{
	return int4(p.x / q, p.y / q, p.z / q, p.w / q);
}
static inline int2 operator/(int p, int2 q)
{
	return int2(p / q.x, p / q.y);
}
static inline int3 operator/(int p, int3 q)
{
	return int3(p / q.x, p / q.y, p / q.z);
}
static inline int4 operator/(int p, int4 q)
{
	return int4(p / q.x, p / q.y, p / q.z, p / q.w);
}
static inline uint2 operator/(uint2 p, uint2 q)
{
	return uint2(p.x / q.x, p.y / q.y);
}
static inline uint3 operator/(uint3 p, uint3 q)
{
	return uint3(p.x / q.x, p.y / q.y, p.z / q.z);
}
static inline uint4 operator/(uint4 p, uint4 q)
{
	return uint4(p.x / q.x, p.y / q.y, p.z / q.z, p.w / q.w);
}
static inline uint2 operator/(uint2 p, uint q)
{
	return uint2(p.x / q, p.y / q);
}
static inline uint3 operator/(uint3 p, uint q)
{
	return uint3(p.x / q, p.y / q, p.z / q);
}
static inline uint4 operator/(uint4 p, uint q)
{
	return uint4(p.x / q, p.y / q, p.z / q, p.w / q);
}
static inline uint2 operator/(uint p, uint2 q)
{
	return uint2(p / q.x, p / q.y);
}
static inline uint3 operator/(uint p, uint3 q)
{
	return uint3(p / q.x, p / q.y, p / q.z);
}
static inline uint4 operator/(uint p, uint4 q)
{
	return uint4(p / q.x, p / q.y, p / q.z, p / q.w);
}
static inline short2 operator/(short2 p, short2 q)
{
	return short2(p.x / q.x, p.y / q.y);
}
static inline short3 operator/(short3 p, short3 q)
{
	return short3(p.x / q.x, p.y / q.y, p.z / q.z);
}
static inline short4 operator/(short4 p, short4 q)
{
	return short4(p.x / q.x, p.y / q.y, p.z / q.z, p.w / q.w);
}
static inline short2 operator/(short2 p, short q)
{
	return short2(p.x / q, p.y / q);
}
static inline short3 operator/(short3 p, short q)
{
	return short3(p.x / q, p.y / q, p.z / q);
}
static inline short4 operator/(short4 p, short q)
{
	return short4(p.x / q, p.y / q, p.z / q, p.w / q);
}
static inline short2 operator/(short p, short2 q)
{
	return short2(p / q.x, p / q.y);
}
static inline short3 operator/(short p, short3 q)
{
	return short3(p / q.x, p / q.y, p / q.z);
}
static inline short4 operator/(short p, short4 q)
{
	return short4(p / q.x, p / q.y, p / q.z, p / q.w);
}
static inline ushort2 operator/(ushort2 p, ushort2 q)
{
	return ushort2(p.x / q.x, p.y / q.y);
}
static inline ushort3 operator/(ushort3 p, ushort3 q)
{
	return ushort3(p.x / q.x, p.y / q.y, p.z / q.z);
}
static inline ushort4 operator/(ushort4 p, ushort4 q)
{
	return ushort4(p.x / q.x, p.y / q.y, p.z / q.z, p.w / q.w);
}
static inline ushort2 operator/(ushort2 p, ushort q)
{
	return ushort2(p.x / q, p.y / q);
}
static inline ushort3 operator/(ushort3 p, ushort q)
{
	return ushort3(p.x / q, p.y / q, p.z / q);
}
static inline ushort4 operator/(ushort4 p, ushort q)
{
	return ushort4(p.x / q, p.y / q, p.z / q, p.w / q);
}
static inline ushort2 operator/(ushort p, ushort2 q)
{
	return ushort2(p / q.x, p / q.y);
}
static inline ushort3 operator/(ushort p, ushort3 q)
{
	return ushort3(p / q.x, p / q.y, p / q.z);
}
static inline ushort4 operator/(ushort p, ushort4 q)
{
	return ushort4(p / q.x, p / q.y, p / q.z, p / q.w);
}
static inline long2 operator/(long2 p, long2 q)
{
	return long2(p.x / q.x, p.y / q.y);
}
static inline long3 operator/(long3 p, long3 q)
{
	return long3(p.x / q.x, p.y / q.y, p.z / q.z);
}
static inline long4 operator/(long4 p, long4 q)
{
	return long4(p.x / q.x, p.y / q.y, p.z / q.z, p.w / q.w);
}
static inline long2 operator/(long2 p, long q)
{
	return long2(p.x / q, p.y / q);
}
static inline long3 operator/(long3 p, long q)
{
	return long3(p.x / q, p.y / q, p.z / q);
}
static inline long4 operator/(long4 p, long q)
{
	return long4(p.x / q, p.y / q, p.z / q, p.w / q);
}
static inline long2 operator/(long p, long2 q)
{
	return long2(p / q.x, p / q.y);
}
static inline long3 operator/(long p, long3 q)
{
	return long3(p / q.x, p / q.y, p / q.z);
}
static inline long4 operator/(long p, long4 q)
{
	return long4(p / q.x, p / q.y, p / q.z, p / q.w);
}
static inline ulong2 operator/(ulong2 p, ulong2 q)
{
	return ulong2(p.x / q.x, p.y / q.y);
}
static inline ulong3 operator/(ulong3 p, ulong3 q)
{
	return ulong3(p.x / q.x, p.y / q.y, p.z / q.z);
}
static inline ulong4 operator/(ulong4 p, ulong4 q)
{
	return ulong4(p.x / q.x, p.y / q.y, p.z / q.z, p.w / q.w);
}
static inline ulong2 operator/(ulong2 p, ulong q)
{
	return ulong2(p.x / q, p.y / q);
}
static inline ulong3 operator/(ulong3 p, ulong q)
{
	return ulong3(p.x / q, p.y / q, p.z / q);
}
static inline ulong4 operator/(ulong4 p, ulong q)
{
	return ulong4(p.x / q, p.y / q, p.z / q, p.w / q);
}
static inline ulong2 operator/(ulong p, ulong2 q)
{
	return ulong2(p / q.x, p / q.y);
}
static inline ulong3 operator/(ulong p, ulong3 q)
{
	return ulong3(p / q.x, p / q.y, p / q.z);
}
static inline ulong4 operator/(ulong p, ulong4 q)
{
	return ulong4(p / q.x, p / q.y, p / q.z, p / q.w);
}
static inline int2 operator%(int2 p, int2 q)
{
	return int2(p.x % q.x, p.y % q.y);
}
static inline int3 operator%(int3 p, int3 q)
{
	return int3(p.x % q.x, p.y % q.y, p.z % q.z);
}
static inline int4 operator%(int4 p, int4 q)
{
	return int4(p.x % q.x, p.y % q.y, p.z % q.z, p.w % q.w);
}
static inline int2 operator%(int2 p, int q)
{
	return int2(p.x % q, p.y % q);
}
static inline int3 operator%(int3 p, int q)
{
	return int3(p.x % q, p.y % q, p.z % q);
}
static inline int4 operator%(int4 p, int q)
{
	return int4(p.x % q, p.y % q, p.z % q, p.w % q);
}
static inline int2 operator%(int p, int2 q)
{
	return int2(p % q.x, p % q.y);
}
static inline int3 operator%(int p, int3 q)
{
	return int3(p % q.x, p % q.y, p % q.z);
}
static inline int4 operator%(int p, int4 q)
{
	return int4(p % q.x, p % q.y, p % q.z, p % q.w);
}
static inline uint2 operator%(uint2 p, uint2 q)
{
	return uint2(p.x % q.x, p.y % q.y);
}
static inline uint3 operator%(uint3 p, uint3 q)
{
	return uint3(p.x % q.x, p.y % q.y, p.z % q.z);
}
static inline uint4 operator%(uint4 p, uint4 q)
{
	return uint4(p.x % q.x, p.y % q.y, p.z % q.z, p.w % q.w);
}
static inline uint2 operator%(uint2 p, uint q)
{
	return uint2(p.x % q, p.y % q);
}
static inline uint3 operator%(uint3 p, uint q)
{
	return uint3(p.x % q, p.y % q, p.z % q);
}
static inline uint4 operator%(uint4 p, uint q)
{
	return uint4(p.x % q, p.y % q, p.z % q, p.w % q);
}
static inline uint2 operator%(uint p, uint2 q)
{
	return uint2(p % q.x, p % q.y);
}
static inline uint3 operator%(uint p, uint3 q)
{
	return uint3(p % q.x, p % q.y, p % q.z);
}
static inline uint4 operator%(uint p, uint4 q)
{
	return uint4(p % q.x, p % q.y, p % q.z, p % q.w);
}
static inline short2 operator%(short2 p, short2 q)
{
	return short2(p.x % q.x, p.y % q.y);
}
static inline short3 operator%(short3 p, short3 q)
{
	return short3(p.x % q.x, p.y % q.y, p.z % q.z);
}
static inline short4 operator%(short4 p, short4 q)
{
	return short4(p.x % q.x, p.y % q.y, p.z % q.z, p.w % q.w);
}
static inline short2 operator%(short2 p, short q)
{
	return short2(p.x % q, p.y % q);
}
static inline short3 operator%(short3 p, short q)
{
	return short3(p.x % q, p.y % q, p.z % q);
}
static inline short4 operator%(short4 p, short q)
{
	return short4(p.x % q, p.y % q, p.z % q, p.w % q);
}
static inline short2 operator%(short p, short2 q)
{
	return short2(p % q.x, p % q.y);
}
static inline short3 operator%(short p, short3 q)
{
	return short3(p % q.x, p % q.y, p % q.z);
}
static inline short4 operator%(short p, short4 q)
{
	return short4(p % q.x, p % q.y, p % q.z, p % q.w);
}
static inline ushort2 operator%(ushort2 p, ushort2 q)
{
	return ushort2(p.x % q.x, p.y % q.y);
}
static inline ushort3 operator%(ushort3 p, ushort3 q)
{
	return ushort3(p.x % q.x, p.y % q.y, p.z % q.z);
}
static inline ushort4 operator%(ushort4 p, ushort4 q)
{
	return ushort4(p.x % q.x, p.y % q.y, p.z % q.z, p.w % q.w);
}
static inline ushort2 operator%(ushort2 p, ushort q)
{
	return ushort2(p.x % q, p.y % q);
}
static inline ushort3 operator%(ushort3 p, ushort q)
{
	return ushort3(p.x % q, p.y % q, p.z % q);
}
static inline ushort4 operator%(ushort4 p, ushort q)
{
	return ushort4(p.x % q, p.y % q, p.z % q, p.w % q);
}
static inline ushort2 operator%(ushort p, ushort2 q)
{
	return ushort2(p % q.x, p % q.y);
}
static inline ushort3 operator%(ushort p, ushort3 q)
{
	return ushort3(p % q.x, p % q.y, p % q.z);
}
static inline ushort4 operator%(ushort p, ushort4 q)
{
	return ushort4(p % q.x, p % q.y, p % q.z, p % q.w);
}
static inline long2 operator%(long2 p, long2 q)
{
	return long2(p.x % q.x, p.y % q.y);
}
static inline long3 operator%(long3 p, long3 q)
{
	return long3(p.x % q.x, p.y % q.y, p.z % q.z);
}
static inline long4 operator%(long4 p, long4 q)
{
	return long4(p.x % q.x, p.y % q.y, p.z % q.z, p.w % q.w);
}
static inline long2 operator%(long2 p, long q)
{
	return long2(p.x % q, p.y % q);
}
static inline long3 operator%(long3 p, long q)
{
	return long3(p.x % q, p.y % q, p.z % q);
}
static inline long4 operator%(long4 p, long q)
{
	return long4(p.x % q, p.y % q, p.z % q, p.w % q);
}
static inline long2 operator%(long p, long2 q)
{
	return long2(p % q.x, p % q.y);
}
static inline long3 operator%(long p, long3 q)
{
	return long3(p % q.x, p % q.y, p % q.z);
}
static inline long4 operator%(long p, long4 q)
{
	return long4(p % q.x, p % q.y, p % q.z, p % q.w);
}
static inline ulong2 operator%(ulong2 p, ulong2 q)
{
	return ulong2(p.x % q.x, p.y % q.y);
}
static inline ulong3 operator%(ulong3 p, ulong3 q)
{
	return ulong3(p.x % q.x, p.y % q.y, p.z % q.z);
}
static inline ulong4 operator%(ulong4 p, ulong4 q)
{
	return ulong4(p.x % q.x, p.y % q.y, p.z % q.z, p.w % q.w);
}
static inline ulong2 operator%(ulong2 p, ulong q)
{
	return ulong2(p.x % q, p.y % q);
}
static inline ulong3 operator%(ulong3 p, ulong q)
{
	return ulong3(p.x % q, p.y % q, p.z % q);
}
static inline ulong4 operator%(ulong4 p, ulong q)
{
	return ulong4(p.x % q, p.y % q, p.z % q, p.w % q);
}
static inline ulong2 operator%(ulong p, ulong2 q)
{
	return ulong2(p % q.x, p % q.y);
}
static inline ulong3 operator%(ulong p, ulong3 q)
{
	return ulong3(p % q.x, p % q.y, p % q.z);
}
static inline ulong4 operator%(ulong p, ulong4 q)
{
	return ulong4(p % q.x, p % q.y, p % q.z, p % q.w);
}
static inline int2 operator&(int2 p, int2 q)
{
	return int2(p.x & q.x, p.y & q.y);
}
static inline int3 operator&(int3 p, int3 q)
{
	return int3(p.x & q.x, p.y & q.y, p.z & q.z);
}
static inline int4 operator&(int4 p, int4 q)
{
	return int4(p.x & q.x, p.y & q.y, p.z & q.z, p.w & q.w);
}
static inline int2 operator&(int2 p, int q)
{
	return int2(p.x & q, p.y & q);
}
static inline int3 operator&(int3 p, int q)
{
	return int3(p.x & q, p.y & q, p.z & q);
}
static inline int4 operator&(int4 p, int q)
{
	return int4(p.x & q, p.y & q, p.z & q, p.w & q);
}
static inline int2 operator&(int p, int2 q)
{
	return int2(p & q.x, p & q.y);
}
static inline int3 operator&(int p, int3 q)
{
	return int3(p & q.x, p & q.y, p & q.z);
}
static inline int4 operator&(int p, int4 q)
{
	return int4(p & q.x, p & q.y, p & q.z, p & q.w);
}
static inline uint2 operator&(uint2 p, uint2 q)
{
	return uint2(p.x & q.x, p.y & q.y);
}
static inline uint3 operator&(uint3 p, uint3 q)
{
	return uint3(p.x & q.x, p.y & q.y, p.z & q.z);
}
static inline uint4 operator&(uint4 p, uint4 q)
{
	return uint4(p.x & q.x, p.y & q.y, p.z & q.z, p.w & q.w);
}
static inline uint2 operator&(uint2 p, uint q)
{
	return uint2(p.x & q, p.y & q);
}
static inline uint3 operator&(uint3 p, uint q)
{
	return uint3(p.x & q, p.y & q, p.z & q);
}
static inline uint4 operator&(uint4 p, uint q)
{
	return uint4(p.x & q, p.y & q, p.z & q, p.w & q);
}
static inline uint2 operator&(uint p, uint2 q)
{
	return uint2(p & q.x, p & q.y);
}
static inline uint3 operator&(uint p, uint3 q)
{
	return uint3(p & q.x, p & q.y, p & q.z);
}
static inline uint4 operator&(uint p, uint4 q)
{
	return uint4(p & q.x, p & q.y, p & q.z, p & q.w);
}
static inline short2 operator&(short2 p, short2 q)
{
	return short2(p.x & q.x, p.y & q.y);
}
static inline short3 operator&(short3 p, short3 q)
{
	return short3(p.x & q.x, p.y & q.y, p.z & q.z);
}
static inline short4 operator&(short4 p, short4 q)
{
	return short4(p.x & q.x, p.y & q.y, p.z & q.z, p.w & q.w);
}
static inline short2 operator&(short2 p, short q)
{
	return short2(p.x & q, p.y & q);
}
static inline short3 operator&(short3 p, short q)
{
	return short3(p.x & q, p.y & q, p.z & q);
}
static inline short4 operator&(short4 p, short q)
{
	return short4(p.x & q, p.y & q, p.z & q, p.w & q);
}
static inline short2 operator&(short p, short2 q)
{
	return short2(p & q.x, p & q.y);
}
static inline short3 operator&(short p, short3 q)
{
	return short3(p & q.x, p & q.y, p & q.z);
}
static inline short4 operator&(short p, short4 q)
{
	return short4(p & q.x, p & q.y, p & q.z, p & q.w);
}
static inline ushort2 operator&(ushort2 p, ushort2 q)
{
	return ushort2(p.x & q.x, p.y & q.y);
}
static inline ushort3 operator&(ushort3 p, ushort3 q)
{
	return ushort3(p.x & q.x, p.y & q.y, p.z & q.z);
}
static inline ushort4 operator&(ushort4 p, ushort4 q)
{
	return ushort4(p.x & q.x, p.y & q.y, p.z & q.z, p.w & q.w);
}
static inline ushort2 operator&(ushort2 p, ushort q)
{
	return ushort2(p.x & q, p.y & q);
}
static inline ushort3 operator&(ushort3 p, ushort q)
{
	return ushort3(p.x & q, p.y & q, p.z & q);
}
static inline ushort4 operator&(ushort4 p, ushort q)
{
	return ushort4(p.x & q, p.y & q, p.z & q, p.w & q);
}
static inline ushort2 operator&(ushort p, ushort2 q)
{
	return ushort2(p & q.x, p & q.y);
}
static inline ushort3 operator&(ushort p, ushort3 q)
{
	return ushort3(p & q.x, p & q.y, p & q.z);
}
static inline ushort4 operator&(ushort p, ushort4 q)
{
	return ushort4(p & q.x, p & q.y, p & q.z, p & q.w);
}
static inline long2 operator&(long2 p, long2 q)
{
	return long2(p.x & q.x, p.y & q.y);
}
static inline long3 operator&(long3 p, long3 q)
{
	return long3(p.x & q.x, p.y & q.y, p.z & q.z);
}
static inline long4 operator&(long4 p, long4 q)
{
	return long4(p.x & q.x, p.y & q.y, p.z & q.z, p.w & q.w);
}
static inline long2 operator&(long2 p, long q)
{
	return long2(p.x & q, p.y & q);
}
static inline long3 operator&(long3 p, long q)
{
	return long3(p.x & q, p.y & q, p.z & q);
}
static inline long4 operator&(long4 p, long q)
{
	return long4(p.x & q, p.y & q, p.z & q, p.w & q);
}
static inline long2 operator&(long p, long2 q)
{
	return long2(p & q.x, p & q.y);
}
static inline long3 operator&(long p, long3 q)
{
	return long3(p & q.x, p & q.y, p & q.z);
}
static inline long4 operator&(long p, long4 q)
{
	return long4(p & q.x, p & q.y, p & q.z, p & q.w);
}
static inline ulong2 operator&(ulong2 p, ulong2 q)
{
	return ulong2(p.x & q.x, p.y & q.y);
}
static inline ulong3 operator&(ulong3 p, ulong3 q)
{
	return ulong3(p.x & q.x, p.y & q.y, p.z & q.z);
}
static inline ulong4 operator&(ulong4 p, ulong4 q)
{
	return ulong4(p.x & q.x, p.y & q.y, p.z & q.z, p.w & q.w);
}
static inline ulong2 operator&(ulong2 p, ulong q)
{
	return ulong2(p.x & q, p.y & q);
}
static inline ulong3 operator&(ulong3 p, ulong q)
{
	return ulong3(p.x & q, p.y & q, p.z & q);
}
static inline ulong4 operator&(ulong4 p, ulong q)
{
	return ulong4(p.x & q, p.y & q, p.z & q, p.w & q);
}
static inline ulong2 operator&(ulong p, ulong2 q)
{
	return ulong2(p & q.x, p & q.y);
}
static inline ulong3 operator&(ulong p, ulong3 q)
{
	return ulong3(p & q.x, p & q.y, p & q.z);
}
static inline ulong4 operator&(ulong p, ulong4 q)
{
	return ulong4(p & q.x, p & q.y, p & q.z, p & q.w);
}
static inline int2 operator|(int2 p, int2 q)
{
	return int2(p.x | q.x, p.y | q.y);
}
static inline int3 operator|(int3 p, int3 q)
{
	return int3(p.x | q.x, p.y | q.y, p.z | q.z);
}
static inline int4 operator|(int4 p, int4 q)
{
	return int4(p.x | q.x, p.y | q.y, p.z | q.z, p.w | q.w);
}
static inline int2 operator|(int2 p, int q)
{
	return int2(p.x | q, p.y | q);
}
static inline int3 operator|(int3 p, int q)
{
	return int3(p.x | q, p.y | q, p.z | q);
}
static inline int4 operator|(int4 p, int q)
{
	return int4(p.x | q, p.y | q, p.z | q, p.w | q);
}
static inline int2 operator|(int p, int2 q)
{
	return int2(p | q.x, p | q.y);
}
static inline int3 operator|(int p, int3 q)
{
	return int3(p | q.x, p | q.y, p | q.z);
}
static inline int4 operator|(int p, int4 q)
{
	return int4(p | q.x, p | q.y, p | q.z, p | q.w);
}
static inline uint2 operator|(uint2 p, uint2 q)
{
	return uint2(p.x | q.x, p.y | q.y);
}
static inline uint3 operator|(uint3 p, uint3 q)
{
	return uint3(p.x | q.x, p.y | q.y, p.z | q.z);
}
static inline uint4 operator|(uint4 p, uint4 q)
{
	return uint4(p.x | q.x, p.y | q.y, p.z | q.z, p.w | q.w);
}
static inline uint2 operator|(uint2 p, uint q)
{
	return uint2(p.x | q, p.y | q);
}
static inline uint3 operator|(uint3 p, uint q)
{
	return uint3(p.x | q, p.y | q, p.z | q);
}
static inline uint4 operator|(uint4 p, uint q)
{
	return uint4(p.x | q, p.y | q, p.z | q, p.w | q);
}
static inline uint2 operator|(uint p, uint2 q)
{
	return uint2(p | q.x, p | q.y);
}
static inline uint3 operator|(uint p, uint3 q)
{
	return uint3(p | q.x, p | q.y, p | q.z);
}
static inline uint4 operator|(uint p, uint4 q)
{
	return uint4(p | q.x, p | q.y, p | q.z, p | q.w);
}
static inline short2 operator|(short2 p, short2 q)
{
	return short2(p.x | q.x, p.y | q.y);
}
static inline short3 operator|(short3 p, short3 q)
{
	return short3(p.x | q.x, p.y | q.y, p.z | q.z);
}
static inline short4 operator|(short4 p, short4 q)
{
	return short4(p.x | q.x, p.y | q.y, p.z | q.z, p.w | q.w);
}
static inline short2 operator|(short2 p, short q)
{
	return short2(p.x | q, p.y | q);
}
static inline short3 operator|(short3 p, short q)
{
	return short3(p.x | q, p.y | q, p.z | q);
}
static inline short4 operator|(short4 p, short q)
{
	return short4(p.x | q, p.y | q, p.z | q, p.w | q);
}
static inline short2 operator|(short p, short2 q)
{
	return short2(p | q.x, p | q.y);
}
static inline short3 operator|(short p, short3 q)
{
	return short3(p | q.x, p | q.y, p | q.z);
}
static inline short4 operator|(short p, short4 q)
{
	return short4(p | q.x, p | q.y, p | q.z, p | q.w);
}
static inline ushort2 operator|(ushort2 p, ushort2 q)
{
	return ushort2(p.x | q.x, p.y | q.y);
}
static inline ushort3 operator|(ushort3 p, ushort3 q)
{
	return ushort3(p.x | q.x, p.y | q.y, p.z | q.z);
}
static inline ushort4 operator|(ushort4 p, ushort4 q)
{
	return ushort4(p.x | q.x, p.y | q.y, p.z | q.z, p.w | q.w);
}
static inline ushort2 operator|(ushort2 p, ushort q)
{
	return ushort2(p.x | q, p.y | q);
}
static inline ushort3 operator|(ushort3 p, ushort q)
{
	return ushort3(p.x | q, p.y | q, p.z | q);
}
static inline ushort4 operator|(ushort4 p, ushort q)
{
	return ushort4(p.x | q, p.y | q, p.z | q, p.w | q);
}
static inline ushort2 operator|(ushort p, ushort2 q)
{
	return ushort2(p | q.x, p | q.y);
}
static inline ushort3 operator|(ushort p, ushort3 q)
{
	return ushort3(p | q.x, p | q.y, p | q.z);
}
static inline ushort4 operator|(ushort p, ushort4 q)
{
	return ushort4(p | q.x, p | q.y, p | q.z, p | q.w);
}
static inline long2 operator|(long2 p, long2 q)
{
	return long2(p.x | q.x, p.y | q.y);
}
static inline long3 operator|(long3 p, long3 q)
{
	return long3(p.x | q.x, p.y | q.y, p.z | q.z);
}
static inline long4 operator|(long4 p, long4 q)
{
	return long4(p.x | q.x, p.y | q.y, p.z | q.z, p.w | q.w);
}
static inline long2 operator|(long2 p, long q)
{
	return long2(p.x | q, p.y | q);
}
static inline long3 operator|(long3 p, long q)
{
	return long3(p.x | q, p.y | q, p.z | q);
}
static inline long4 operator|(long4 p, long q)
{
	return long4(p.x | q, p.y | q, p.z | q, p.w | q);
}
static inline long2 operator|(long p, long2 q)
{
	return long2(p | q.x, p | q.y);
}
static inline long3 operator|(long p, long3 q)
{
	return long3(p | q.x, p | q.y, p | q.z);
}
static inline long4 operator|(long p, long4 q)
{
	return long4(p | q.x, p | q.y, p | q.z, p | q.w);
}
static inline ulong2 operator|(ulong2 p, ulong2 q)
{
	return ulong2(p.x | q.x, p.y | q.y);
}
static inline ulong3 operator|(ulong3 p, ulong3 q)
{
	return ulong3(p.x | q.x, p.y | q.y, p.z | q.z);
}
static inline ulong4 operator|(ulong4 p, ulong4 q)
{
	return ulong4(p.x | q.x, p.y | q.y, p.z | q.z, p.w | q.w);
}
static inline ulong2 operator|(ulong2 p, ulong q)
{
	return ulong2(p.x | q, p.y | q);
}
static inline ulong3 operator|(ulong3 p, ulong q)
{
	return ulong3(p.x | q, p.y | q, p.z | q);
}
static inline ulong4 operator|(ulong4 p, ulong q)
{
	return ulong4(p.x | q, p.y | q, p.z | q, p.w | q);
}
static inline ulong2 operator|(ulong p, ulong2 q)
{
	return ulong2(p | q.x, p | q.y);
}
static inline ulong3 operator|(ulong p, ulong3 q)
{
	return ulong3(p | q.x, p | q.y, p | q.z);
}
static inline ulong4 operator|(ulong p, ulong4 q)
{
	return ulong4(p | q.x, p | q.y, p | q.z, p | q.w);
}
static inline int2 operator^(int2 p, int2 q)
{
	return int2(p.x ^ q.x, p.y ^ q.y);
}
static inline int3 operator^(int3 p, int3 q)
{
	return int3(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z);
}
static inline int4 operator^(int4 p, int4 q)
{
	return int4(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z, p.w ^ q.w);
}
static inline int2 operator^(int2 p, int q)
{
	return int2(p.x ^ q, p.y ^ q);
}
static inline int3 operator^(int3 p, int q)
{
	return int3(p.x ^ q, p.y ^ q, p.z ^ q);
}
static inline int4 operator^(int4 p, int q)
{
	return int4(p.x ^ q, p.y ^ q, p.z ^ q, p.w ^ q);
}
static inline int2 operator^(int p, int2 q)
{
	return int2(p ^ q.x, p ^ q.y);
}
static inline int3 operator^(int p, int3 q)
{
	return int3(p ^ q.x, p ^ q.y, p ^ q.z);
}
static inline int4 operator^(int p, int4 q)
{
	return int4(p ^ q.x, p ^ q.y, p ^ q.z, p ^ q.w);
}
static inline uint2 operator^(uint2 p, uint2 q)
{
	return uint2(p.x ^ q.x, p.y ^ q.y);
}
static inline uint3 operator^(uint3 p, uint3 q)
{
	return uint3(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z);
}
static inline uint4 operator^(uint4 p, uint4 q)
{
	return uint4(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z, p.w ^ q.w);
}
static inline uint2 operator^(uint2 p, uint q)
{
	return uint2(p.x ^ q, p.y ^ q);
}
static inline uint3 operator^(uint3 p, uint q)
{
	return uint3(p.x ^ q, p.y ^ q, p.z ^ q);
}
static inline uint4 operator^(uint4 p, uint q)
{
	return uint4(p.x ^ q, p.y ^ q, p.z ^ q, p.w ^ q);
}
static inline uint2 operator^(uint p, uint2 q)
{
	return uint2(p ^ q.x, p ^ q.y);
}
static inline uint3 operator^(uint p, uint3 q)
{
	return uint3(p ^ q.x, p ^ q.y, p ^ q.z);
}
static inline uint4 operator^(uint p, uint4 q)
{
	return uint4(p ^ q.x, p ^ q.y, p ^ q.z, p ^ q.w);
}
static inline short2 operator^(short2 p, short2 q)
{
	return short2(p.x ^ q.x, p.y ^ q.y);
}
static inline short3 operator^(short3 p, short3 q)
{
	return short3(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z);
}
static inline short4 operator^(short4 p, short4 q)
{
	return short4(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z, p.w ^ q.w);
}
static inline short2 operator^(short2 p, short q)
{
	return short2(p.x ^ q, p.y ^ q);
}
static inline short3 operator^(short3 p, short q)
{
	return short3(p.x ^ q, p.y ^ q, p.z ^ q);
}
static inline short4 operator^(short4 p, short q)
{
	return short4(p.x ^ q, p.y ^ q, p.z ^ q, p.w ^ q);
}
static inline short2 operator^(short p, short2 q)
{
	return short2(p ^ q.x, p ^ q.y);
}
static inline short3 operator^(short p, short3 q)
{
	return short3(p ^ q.x, p ^ q.y, p ^ q.z);
}
static inline short4 operator^(short p, short4 q)
{
	return short4(p ^ q.x, p ^ q.y, p ^ q.z, p ^ q.w);
}
static inline ushort2 operator^(ushort2 p, ushort2 q)
{
	return ushort2(p.x ^ q.x, p.y ^ q.y);
}
static inline ushort3 operator^(ushort3 p, ushort3 q)
{
	return ushort3(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z);
}
static inline ushort4 operator^(ushort4 p, ushort4 q)
{
	return ushort4(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z, p.w ^ q.w);
}
static inline ushort2 operator^(ushort2 p, ushort q)
{
	return ushort2(p.x ^ q, p.y ^ q);
}
static inline ushort3 operator^(ushort3 p, ushort q)
{
	return ushort3(p.x ^ q, p.y ^ q, p.z ^ q);
}
static inline ushort4 operator^(ushort4 p, ushort q)
{
	return ushort4(p.x ^ q, p.y ^ q, p.z ^ q, p.w ^ q);
}
static inline ushort2 operator^(ushort p, ushort2 q)
{
	return ushort2(p ^ q.x, p ^ q.y);
}
static inline ushort3 operator^(ushort p, ushort3 q)
{
	return ushort3(p ^ q.x, p ^ q.y, p ^ q.z);
}
static inline ushort4 operator^(ushort p, ushort4 q)
{
	return ushort4(p ^ q.x, p ^ q.y, p ^ q.z, p ^ q.w);
}
static inline long2 operator^(long2 p, long2 q)
{
	return long2(p.x ^ q.x, p.y ^ q.y);
}
static inline long3 operator^(long3 p, long3 q)
{
	return long3(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z);
}
static inline long4 operator^(long4 p, long4 q)
{
	return long4(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z, p.w ^ q.w);
}
static inline long2 operator^(long2 p, long q)
{
	return long2(p.x ^ q, p.y ^ q);
}
static inline long3 operator^(long3 p, long q)
{
	return long3(p.x ^ q, p.y ^ q, p.z ^ q);
}
static inline long4 operator^(long4 p, long q)
{
	return long4(p.x ^ q, p.y ^ q, p.z ^ q, p.w ^ q);
}
static inline long2 operator^(long p, long2 q)
{
	return long2(p ^ q.x, p ^ q.y);
}
static inline long3 operator^(long p, long3 q)
{
	return long3(p ^ q.x, p ^ q.y, p ^ q.z);
}
static inline long4 operator^(long p, long4 q)
{
	return long4(p ^ q.x, p ^ q.y, p ^ q.z, p ^ q.w);
}
static inline ulong2 operator^(ulong2 p, ulong2 q)
{
	return ulong2(p.x ^ q.x, p.y ^ q.y);
}
static inline ulong3 operator^(ulong3 p, ulong3 q)
{
	return ulong3(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z);
}
static inline ulong4 operator^(ulong4 p, ulong4 q)
{
	return ulong4(p.x ^ q.x, p.y ^ q.y, p.z ^ q.z, p.w ^ q.w);
}
static inline ulong2 operator^(ulong2 p, ulong q)
{
	return ulong2(p.x ^ q, p.y ^ q);
}
static inline ulong3 operator^(ulong3 p, ulong q)
{
	return ulong3(p.x ^ q, p.y ^ q, p.z ^ q);
}
static inline ulong4 operator^(ulong4 p, ulong q)
{
	return ulong4(p.x ^ q, p.y ^ q, p.z ^ q, p.w ^ q);
}
static inline ulong2 operator^(ulong p, ulong2 q)
{
	return ulong2(p ^ q.x, p ^ q.y);
}
static inline ulong3 operator^(ulong p, ulong3 q)
{
	return ulong3(p ^ q.x, p ^ q.y, p ^ q.z);
}
static inline ulong4 operator^(ulong p, ulong4 q)
{
	return ulong4(p ^ q.x, p ^ q.y, p ^ q.z, p ^ q.w);
}
static inline int2 operator<<(int2 p, int2 q)
{
	return int2(p.x << q.x, p.y << q.y);
}
static inline int3 operator<<(int3 p, int3 q)
{
	return int3(p.x << q.x, p.y << q.y, p.z << q.z);
}
static inline int4 operator<<(int4 p, int4 q)
{
	return int4(p.x << q.x, p.y << q.y, p.z << q.z, p.w << q.w);
}
static inline int2 operator<<(int2 p, int q)
{
	return int2(p.x << q, p.y << q);
}
static inline int3 operator<<(int3 p, int q)
{
	return int3(p.x << q, p.y << q, p.z << q);
}
static inline int4 operator<<(int4 p, int q)
{
	return int4(p.x << q, p.y << q, p.z << q, p.w << q);
}
static inline int2 operator<<(int p, int2 q)
{
	return int2(p << q.x, p << q.y);
}
static inline int3 operator<<(int p, int3 q)
{
	return int3(p << q.x, p << q.y, p << q.z);
}
static inline int4 operator<<(int p, int4 q)
{
	return int4(p << q.x, p << q.y, p << q.z, p << q.w);
}
static inline uint2 operator<<(uint2 p, uint2 q)
{
	return uint2(p.x << q.x, p.y << q.y);
}
static inline uint3 operator<<(uint3 p, uint3 q)
{
	return uint3(p.x << q.x, p.y << q.y, p.z << q.z);
}
static inline uint4 operator<<(uint4 p, uint4 q)
{
	return uint4(p.x << q.x, p.y << q.y, p.z << q.z, p.w << q.w);
}
static inline uint2 operator<<(uint2 p, uint q)
{
	return uint2(p.x << q, p.y << q);
}
static inline uint3 operator<<(uint3 p, uint q)
{
	return uint3(p.x << q, p.y << q, p.z << q);
}
static inline uint4 operator<<(uint4 p, uint q)
{
	return uint4(p.x << q, p.y << q, p.z << q, p.w << q);
}
static inline uint2 operator<<(uint p, uint2 q)
{
	return uint2(p << q.x, p << q.y);
}
static inline uint3 operator<<(uint p, uint3 q)
{
	return uint3(p << q.x, p << q.y, p << q.z);
}
static inline uint4 operator<<(uint p, uint4 q)
{
	return uint4(p << q.x, p << q.y, p << q.z, p << q.w);
}
static inline short2 operator<<(short2 p, short2 q)
{
	return short2(p.x << q.x, p.y << q.y);
}
static inline short3 operator<<(short3 p, short3 q)
{
	return short3(p.x << q.x, p.y << q.y, p.z << q.z);
}
static inline short4 operator<<(short4 p, short4 q)
{
	return short4(p.x << q.x, p.y << q.y, p.z << q.z, p.w << q.w);
}
static inline short2 operator<<(short2 p, short q)
{
	return short2(p.x << q, p.y << q);
}
static inline short3 operator<<(short3 p, short q)
{
	return short3(p.x << q, p.y << q, p.z << q);
}
static inline short4 operator<<(short4 p, short q)
{
	return short4(p.x << q, p.y << q, p.z << q, p.w << q);
}
static inline short2 operator<<(short p, short2 q)
{
	return short2(p << q.x, p << q.y);
}
static inline short3 operator<<(short p, short3 q)
{
	return short3(p << q.x, p << q.y, p << q.z);
}
static inline short4 operator<<(short p, short4 q)
{
	return short4(p << q.x, p << q.y, p << q.z, p << q.w);
}
static inline ushort2 operator<<(ushort2 p, ushort2 q)
{
	return ushort2(p.x << q.x, p.y << q.y);
}
static inline ushort3 operator<<(ushort3 p, ushort3 q)
{
	return ushort3(p.x << q.x, p.y << q.y, p.z << q.z);
}
static inline ushort4 operator<<(ushort4 p, ushort4 q)
{
	return ushort4(p.x << q.x, p.y << q.y, p.z << q.z, p.w << q.w);
}
static inline ushort2 operator<<(ushort2 p, ushort q)
{
	return ushort2(p.x << q, p.y << q);
}
static inline ushort3 operator<<(ushort3 p, ushort q)
{
	return ushort3(p.x << q, p.y << q, p.z << q);
}
static inline ushort4 operator<<(ushort4 p, ushort q)
{
	return ushort4(p.x << q, p.y << q, p.z << q, p.w << q);
}
static inline ushort2 operator<<(ushort p, ushort2 q)
{
	return ushort2(p << q.x, p << q.y);
}
static inline ushort3 operator<<(ushort p, ushort3 q)
{
	return ushort3(p << q.x, p << q.y, p << q.z);
}
static inline ushort4 operator<<(ushort p, ushort4 q)
{
	return ushort4(p << q.x, p << q.y, p << q.z, p << q.w);
}
static inline long2 operator<<(long2 p, long2 q)
{
	return long2(p.x << q.x, p.y << q.y);
}
static inline long3 operator<<(long3 p, long3 q)
{
	return long3(p.x << q.x, p.y << q.y, p.z << q.z);
}
static inline long4 operator<<(long4 p, long4 q)
{
	return long4(p.x << q.x, p.y << q.y, p.z << q.z, p.w << q.w);
}
static inline long2 operator<<(long2 p, long q)
{
	return long2(p.x << q, p.y << q);
}
static inline long3 operator<<(long3 p, long q)
{
	return long3(p.x << q, p.y << q, p.z << q);
}
static inline long4 operator<<(long4 p, long q)
{
	return long4(p.x << q, p.y << q, p.z << q, p.w << q);
}
static inline long2 operator<<(long p, long2 q)
{
	return long2(p << q.x, p << q.y);
}
static inline long3 operator<<(long p, long3 q)
{
	return long3(p << q.x, p << q.y, p << q.z);
}
static inline long4 operator<<(long p, long4 q)
{
	return long4(p << q.x, p << q.y, p << q.z, p << q.w);
}
static inline ulong2 operator<<(ulong2 p, ulong2 q)
{
	return ulong2(p.x << q.x, p.y << q.y);
}
static inline ulong3 operator<<(ulong3 p, ulong3 q)
{
	return ulong3(p.x << q.x, p.y << q.y, p.z << q.z);
}
static inline ulong4 operator<<(ulong4 p, ulong4 q)
{
	return ulong4(p.x << q.x, p.y << q.y, p.z << q.z, p.w << q.w);
}
static inline ulong2 operator<<(ulong2 p, ulong q)
{
	return ulong2(p.x << q, p.y << q);
}
static inline ulong3 operator<<(ulong3 p, ulong q)
{
	return ulong3(p.x << q, p.y << q, p.z << q);
}
static inline ulong4 operator<<(ulong4 p, ulong q)
{
	return ulong4(p.x << q, p.y << q, p.z << q, p.w << q);
}
static inline ulong2 operator<<(ulong p, ulong2 q)
{
	return ulong2(p << q.x, p << q.y);
}
static inline ulong3 operator<<(ulong p, ulong3 q)
{
	return ulong3(p << q.x, p << q.y, p << q.z);
}
static inline ulong4 operator<<(ulong p, ulong4 q)
{
	return ulong4(p << q.x, p << q.y, p << q.z, p << q.w);
}
static inline int2 operator>>(int2 p, int2 q)
{
	return int2(p.x >> q.x, p.y >> q.y);
}
static inline int3 operator>>(int3 p, int3 q)
{
	return int3(p.x >> q.x, p.y >> q.y, p.z >> q.z);
}
static inline int4 operator>>(int4 p, int4 q)
{
	return int4(p.x >> q.x, p.y >> q.y, p.z >> q.z, p.w >> q.w);
}
static inline int2 operator>>(int2 p, int q)
{
	return int2(p.x >> q, p.y >> q);
}
static inline int3 operator>>(int3 p, int q)
{
	return int3(p.x >> q, p.y >> q, p.z >> q);
}
static inline int4 operator>>(int4 p, int q)
{
	return int4(p.x >> q, p.y >> q, p.z >> q, p.w >> q);
}
static inline int2 operator>>(int p, int2 q)
{
	return int2(p >> q.x, p >> q.y);
}
static inline int3 operator>>(int p, int3 q)
{
	return int3(p >> q.x, p >> q.y, p >> q.z);
}
static inline int4 operator>>(int p, int4 q)
{
	return int4(p >> q.x, p >> q.y, p >> q.z, p >> q.w);
}
static inline uint2 operator>>(uint2 p, uint2 q)
{
	return uint2(p.x >> q.x, p.y >> q.y);
}
static inline uint3 operator>>(uint3 p, uint3 q)
{
	return uint3(p.x >> q.x, p.y >> q.y, p.z >> q.z);
}
static inline uint4 operator>>(uint4 p, uint4 q)
{
	return uint4(p.x >> q.x, p.y >> q.y, p.z >> q.z, p.w >> q.w);
}
static inline uint2 operator>>(uint2 p, uint q)
{
	return uint2(p.x >> q, p.y >> q);
}
static inline uint3 operator>>(uint3 p, uint q)
{
	return uint3(p.x >> q, p.y >> q, p.z >> q);
}
static inline uint4 operator>>(uint4 p, uint q)
{
	return uint4(p.x >> q, p.y >> q, p.z >> q, p.w >> q);
}
static inline uint2 operator>>(uint p, uint2 q)
{
	return uint2(p >> q.x, p >> q.y);
}
static inline uint3 operator>>(uint p, uint3 q)
{
	return uint3(p >> q.x, p >> q.y, p >> q.z);
}
static inline uint4 operator>>(uint p, uint4 q)
{
	return uint4(p >> q.x, p >> q.y, p >> q.z, p >> q.w);
}
static inline short2 operator>>(short2 p, short2 q)
{
	return short2(p.x >> q.x, p.y >> q.y);
}
static inline short3 operator>>(short3 p, short3 q)
{
	return short3(p.x >> q.x, p.y >> q.y, p.z >> q.z);
}
static inline short4 operator>>(short4 p, short4 q)
{
	return short4(p.x >> q.x, p.y >> q.y, p.z >> q.z, p.w >> q.w);
}
static inline short2 operator>>(short2 p, short q)
{
	return short2(p.x >> q, p.y >> q);
}
static inline short3 operator>>(short3 p, short q)
{
	return short3(p.x >> q, p.y >> q, p.z >> q);
}
static inline short4 operator>>(short4 p, short q)
{
	return short4(p.x >> q, p.y >> q, p.z >> q, p.w >> q);
}
static inline short2 operator>>(short p, short2 q)
{
	return short2(p >> q.x, p >> q.y);
}
static inline short3 operator>>(short p, short3 q)
{
	return short3(p >> q.x, p >> q.y, p >> q.z);
}
static inline short4 operator>>(short p, short4 q)
{
	return short4(p >> q.x, p >> q.y, p >> q.z, p >> q.w);
}
static inline ushort2 operator>>(ushort2 p, ushort2 q)
{
	return ushort2(p.x >> q.x, p.y >> q.y);
}
static inline ushort3 operator>>(ushort3 p, ushort3 q)
{
	return ushort3(p.x >> q.x, p.y >> q.y, p.z >> q.z);
}
static inline ushort4 operator>>(ushort4 p, ushort4 q)
{
	return ushort4(p.x >> q.x, p.y >> q.y, p.z >> q.z, p.w >> q.w);
}
static inline ushort2 operator>>(ushort2 p, ushort q)
{
	return ushort2(p.x >> q, p.y >> q);
}
static inline ushort3 operator>>(ushort3 p, ushort q)
{
	return ushort3(p.x >> q, p.y >> q, p.z >> q);
}
static inline ushort4 operator>>(ushort4 p, ushort q)
{
	return ushort4(p.x >> q, p.y >> q, p.z >> q, p.w >> q);
}
static inline ushort2 operator>>(ushort p, ushort2 q)
{
	return ushort2(p >> q.x, p >> q.y);
}
static inline ushort3 operator>>(ushort p, ushort3 q)
{
	return ushort3(p >> q.x, p >> q.y, p >> q.z);
}
static inline ushort4 operator>>(ushort p, ushort4 q)
{
	return ushort4(p >> q.x, p >> q.y, p >> q.z, p >> q.w);
}
static inline long2 operator>>(long2 p, long2 q)
{
	return long2(p.x >> q.x, p.y >> q.y);
}
static inline long3 operator>>(long3 p, long3 q)
{
	return long3(p.x >> q.x, p.y >> q.y, p.z >> q.z);
}
static inline long4 operator>>(long4 p, long4 q)
{
	return long4(p.x >> q.x, p.y >> q.y, p.z >> q.z, p.w >> q.w);
}
static inline long2 operator>>(long2 p, long q)
{
	return long2(p.x >> q, p.y >> q);
}
static inline long3 operator>>(long3 p, long q)
{
	return long3(p.x >> q, p.y >> q, p.z >> q);
}
static inline long4 operator>>(long4 p, long q)
{
	return long4(p.x >> q, p.y >> q, p.z >> q, p.w >> q);
}
static inline long2 operator>>(long p, long2 q)
{
	return long2(p >> q.x, p >> q.y);
}
static inline long3 operator>>(long p, long3 q)
{
	return long3(p >> q.x, p >> q.y, p >> q.z);
}
static inline long4 operator>>(long p, long4 q)
{
	return long4(p >> q.x, p >> q.y, p >> q.z, p >> q.w);
}
static inline ulong2 operator>>(ulong2 p, ulong2 q)
{
	return ulong2(p.x >> q.x, p.y >> q.y);
}
static inline ulong3 operator>>(ulong3 p, ulong3 q)
{
	return ulong3(p.x >> q.x, p.y >> q.y, p.z >> q.z);
}
static inline ulong4 operator>>(ulong4 p, ulong4 q)
{
	return ulong4(p.x >> q.x, p.y >> q.y, p.z >> q.z, p.w >> q.w);
}
static inline ulong2 operator>>(ulong2 p, ulong q)
{
	return ulong2(p.x >> q, p.y >> q);
}
static inline ulong3 operator>>(ulong3 p, ulong q)
{
	return ulong3(p.x >> q, p.y >> q, p.z >> q);
}
static inline ulong4 operator>>(ulong4 p, ulong q)
{
	return ulong4(p.x >> q, p.y >> q, p.z >> q, p.w >> q);
}
static inline ulong2 operator>>(ulong p, ulong2 q)
{
	return ulong2(p >> q.x, p >> q.y);
}
static inline ulong3 operator>>(ulong p, ulong3 q)
{
	return ulong3(p >> q.x, p >> q.y, p >> q.z);
}
static inline ulong4 operator>>(ulong p, ulong4 q)
{
	return ulong4(p >> q.x, p >> q.y, p >> q.z, p >> q.w);
}
static inline int2 operator==(float2 p, float2 q)
{
	return int2(p.x == q.x, p.y == q.y);
}
static inline int3 operator==(float3 p, float3 q)
{
	return int3(p.x == q.x, p.y == q.y, p.z == q.z);
}
static inline int4 operator==(float4 p, float4 q)
{
	return int4(p.x == q.x, p.y == q.y, p.z == q.z, p.w == q.w);
}
static inline int2 operator==(float2 p, float q)
{
	return int2(p.x == q, p.y == q);
}
static inline int3 operator==(float3 p, float q)
{
	return int3(p.x == q, p.y == q, p.z == q);
}
static inline int4 operator==(float4 p, float q)
{
	return int4(p.x == q, p.y == q, p.z == q, p.w == q);
}
static inline int2 operator==(float p, float2 q)
{
	return int2(p == q.x, p == q.y);
}
static inline int3 operator==(float p, float3 q)
{
	return int3(p == q.x, p == q.y, p == q.z);
}
static inline int4 operator==(float p, float4 q)
{
	return int4(p == q.x, p == q.y, p == q.z, p == q.w);
}
static inline long2 operator==(double2 p, double2 q)
{
	return long2(p.x == q.x, p.y == q.y);
}
static inline long3 operator==(double3 p, double3 q)
{
	return long3(p.x == q.x, p.y == q.y, p.z == q.z);
}
static inline long4 operator==(double4 p, double4 q)
{
	return long4(p.x == q.x, p.y == q.y, p.z == q.z, p.w == q.w);
}
static inline long2 operator==(double2 p, double q)
{
	return long2(p.x == q, p.y == q);
}
static inline long3 operator==(double3 p, double q)
{
	return long3(p.x == q, p.y == q, p.z == q);
}
static inline long4 operator==(double4 p, double q)
{
	return long4(p.x == q, p.y == q, p.z == q, p.w == q);
}
static inline long2 operator==(double p, double2 q)
{
	return long2(p == q.x, p == q.y);
}
static inline long3 operator==(double p, double3 q)
{
	return long3(p == q.x, p == q.y, p == q.z);
}
static inline long4 operator==(double p, double4 q)
{
	return long4(p == q.x, p == q.y, p == q.z, p == q.w);
}
static inline int2 operator==(int2 p, int2 q)
{
	return int2(p.x == q.x, p.y == q.y);
}
static inline int3 operator==(int3 p, int3 q)
{
	return int3(p.x == q.x, p.y == q.y, p.z == q.z);
}
static inline int4 operator==(int4 p, int4 q)
{
	return int4(p.x == q.x, p.y == q.y, p.z == q.z, p.w == q.w);
}
static inline int2 operator==(int2 p, int q)
{
	return int2(p.x == q, p.y == q);
}
static inline int3 operator==(int3 p, int q)
{
	return int3(p.x == q, p.y == q, p.z == q);
}
static inline int4 operator==(int4 p, int q)
{
	return int4(p.x == q, p.y == q, p.z == q, p.w == q);
}
static inline int2 operator==(int p, int2 q)
{
	return int2(p == q.x, p == q.y);
}
static inline int3 operator==(int p, int3 q)
{
	return int3(p == q.x, p == q.y, p == q.z);
}
static inline int4 operator==(int p, int4 q)
{
	return int4(p == q.x, p == q.y, p == q.z, p == q.w);
}
static inline int2 operator==(uint2 p, uint2 q)
{
	return int2(p.x == q.x, p.y == q.y);
}
static inline int3 operator==(uint3 p, uint3 q)
{
	return int3(p.x == q.x, p.y == q.y, p.z == q.z);
}
static inline int4 operator==(uint4 p, uint4 q)
{
	return int4(p.x == q.x, p.y == q.y, p.z == q.z, p.w == q.w);
}
static inline int2 operator==(uint2 p, uint q)
{
	return int2(p.x == q, p.y == q);
}
static inline int3 operator==(uint3 p, uint q)
{
	return int3(p.x == q, p.y == q, p.z == q);
}
static inline int4 operator==(uint4 p, uint q)
{
	return int4(p.x == q, p.y == q, p.z == q, p.w == q);
}
static inline int2 operator==(uint p, uint2 q)
{
	return int2(p == q.x, p == q.y);
}
static inline int3 operator==(uint p, uint3 q)
{
	return int3(p == q.x, p == q.y, p == q.z);
}
static inline int4 operator==(uint p, uint4 q)
{
	return int4(p == q.x, p == q.y, p == q.z, p == q.w);
}
static inline short2 operator==(short2 p, short2 q)
{
	return short2(p.x == q.x, p.y == q.y);
}
static inline short3 operator==(short3 p, short3 q)
{
	return short3(p.x == q.x, p.y == q.y, p.z == q.z);
}
static inline short4 operator==(short4 p, short4 q)
{
	return short4(p.x == q.x, p.y == q.y, p.z == q.z, p.w == q.w);
}
static inline short2 operator==(short2 p, short q)
{
	return short2(p.x == q, p.y == q);
}
static inline short3 operator==(short3 p, short q)
{
	return short3(p.x == q, p.y == q, p.z == q);
}
static inline short4 operator==(short4 p, short q)
{
	return short4(p.x == q, p.y == q, p.z == q, p.w == q);
}
static inline short2 operator==(short p, short2 q)
{
	return short2(p == q.x, p == q.y);
}
static inline short3 operator==(short p, short3 q)
{
	return short3(p == q.x, p == q.y, p == q.z);
}
static inline short4 operator==(short p, short4 q)
{
	return short4(p == q.x, p == q.y, p == q.z, p == q.w);
}
static inline short2 operator==(ushort2 p, ushort2 q)
{
	return short2(p.x == q.x, p.y == q.y);
}
static inline short3 operator==(ushort3 p, ushort3 q)
{
	return short3(p.x == q.x, p.y == q.y, p.z == q.z);
}
static inline short4 operator==(ushort4 p, ushort4 q)
{
	return short4(p.x == q.x, p.y == q.y, p.z == q.z, p.w == q.w);
}
static inline short2 operator==(ushort2 p, ushort q)
{
	return short2(p.x == q, p.y == q);
}
static inline short3 operator==(ushort3 p, ushort q)
{
	return short3(p.x == q, p.y == q, p.z == q);
}
static inline short4 operator==(ushort4 p, ushort q)
{
	return short4(p.x == q, p.y == q, p.z == q, p.w == q);
}
static inline short2 operator==(ushort p, ushort2 q)
{
	return short2(p == q.x, p == q.y);
}
static inline short3 operator==(ushort p, ushort3 q)
{
	return short3(p == q.x, p == q.y, p == q.z);
}
static inline short4 operator==(ushort p, ushort4 q)
{
	return short4(p == q.x, p == q.y, p == q.z, p == q.w);
}
static inline long2 operator==(long2 p, long2 q)
{
	return long2(p.x == q.x, p.y == q.y);
}
static inline long3 operator==(long3 p, long3 q)
{
	return long3(p.x == q.x, p.y == q.y, p.z == q.z);
}
static inline long4 operator==(long4 p, long4 q)
{
	return long4(p.x == q.x, p.y == q.y, p.z == q.z, p.w == q.w);
}
static inline long2 operator==(long2 p, long q)
{
	return long2(p.x == q, p.y == q);
}
static inline long3 operator==(long3 p, long q)
{
	return long3(p.x == q, p.y == q, p.z == q);
}
static inline long4 operator==(long4 p, long q)
{
	return long4(p.x == q, p.y == q, p.z == q, p.w == q);
}
static inline long2 operator==(long p, long2 q)
{
	return long2(p == q.x, p == q.y);
}
static inline long3 operator==(long p, long3 q)
{
	return long3(p == q.x, p == q.y, p == q.z);
}
static inline long4 operator==(long p, long4 q)
{
	return long4(p == q.x, p == q.y, p == q.z, p == q.w);
}
static inline long2 operator==(ulong2 p, ulong2 q)
{
	return long2(p.x == q.x, p.y == q.y);
}
static inline long3 operator==(ulong3 p, ulong3 q)
{
	return long3(p.x == q.x, p.y == q.y, p.z == q.z);
}
static inline long4 operator==(ulong4 p, ulong4 q)
{
	return long4(p.x == q.x, p.y == q.y, p.z == q.z, p.w == q.w);
}
static inline long2 operator==(ulong2 p, ulong q)
{
	return long2(p.x == q, p.y == q);
}
static inline long3 operator==(ulong3 p, ulong q)
{
	return long3(p.x == q, p.y == q, p.z == q);
}
static inline long4 operator==(ulong4 p, ulong q)
{
	return long4(p.x == q, p.y == q, p.z == q, p.w == q);
}
static inline long2 operator==(ulong p, ulong2 q)
{
	return long2(p == q.x, p == q.y);
}
static inline long3 operator==(ulong p, ulong3 q)
{
	return long3(p == q.x, p == q.y, p == q.z);
}
static inline long4 operator==(ulong p, ulong4 q)
{
	return long4(p == q.x, p == q.y, p == q.z, p == q.w);
}
static inline int2 operator!=(float2 p, float2 q)
{
	return int2(p.x != q.x, p.y != q.y);
}
static inline int3 operator!=(float3 p, float3 q)
{
	return int3(p.x != q.x, p.y != q.y, p.z != q.z);
}
static inline int4 operator!=(float4 p, float4 q)
{
	return int4(p.x != q.x, p.y != q.y, p.z != q.z, p.w != q.w);
}
static inline int2 operator!=(float2 p, float q)
{
	return int2(p.x != q, p.y != q);
}
static inline int3 operator!=(float3 p, float q)
{
	return int3(p.x != q, p.y != q, p.z != q);
}
static inline int4 operator!=(float4 p, float q)
{
	return int4(p.x != q, p.y != q, p.z != q, p.w != q);
}
static inline int2 operator!=(float p, float2 q)
{
	return int2(p != q.x, p != q.y);
}
static inline int3 operator!=(float p, float3 q)
{
	return int3(p != q.x, p != q.y, p != q.z);
}
static inline int4 operator!=(float p, float4 q)
{
	return int4(p != q.x, p != q.y, p != q.z, p != q.w);
}
static inline long2 operator!=(double2 p, double2 q)
{
	return long2(p.x != q.x, p.y != q.y);
}
static inline long3 operator!=(double3 p, double3 q)
{
	return long3(p.x != q.x, p.y != q.y, p.z != q.z);
}
static inline long4 operator!=(double4 p, double4 q)
{
	return long4(p.x != q.x, p.y != q.y, p.z != q.z, p.w != q.w);
}
static inline long2 operator!=(double2 p, double q)
{
	return long2(p.x != q, p.y != q);
}
static inline long3 operator!=(double3 p, double q)
{
	return long3(p.x != q, p.y != q, p.z != q);
}
static inline long4 operator!=(double4 p, double q)
{
	return long4(p.x != q, p.y != q, p.z != q, p.w != q);
}
static inline long2 operator!=(double p, double2 q)
{
	return long2(p != q.x, p != q.y);
}
static inline long3 operator!=(double p, double3 q)
{
	return long3(p != q.x, p != q.y, p != q.z);
}
static inline long4 operator!=(double p, double4 q)
{
	return long4(p != q.x, p != q.y, p != q.z, p != q.w);
}
static inline int2 operator!=(int2 p, int2 q)
{
	return int2(p.x != q.x, p.y != q.y);
}
static inline int3 operator!=(int3 p, int3 q)
{
	return int3(p.x != q.x, p.y != q.y, p.z != q.z);
}
static inline int4 operator!=(int4 p, int4 q)
{
	return int4(p.x != q.x, p.y != q.y, p.z != q.z, p.w != q.w);
}
static inline int2 operator!=(int2 p, int q)
{
	return int2(p.x != q, p.y != q);
}
static inline int3 operator!=(int3 p, int q)
{
	return int3(p.x != q, p.y != q, p.z != q);
}
static inline int4 operator!=(int4 p, int q)
{
	return int4(p.x != q, p.y != q, p.z != q, p.w != q);
}
static inline int2 operator!=(int p, int2 q)
{
	return int2(p != q.x, p != q.y);
}
static inline int3 operator!=(int p, int3 q)
{
	return int3(p != q.x, p != q.y, p != q.z);
}
static inline int4 operator!=(int p, int4 q)
{
	return int4(p != q.x, p != q.y, p != q.z, p != q.w);
}
static inline int2 operator!=(uint2 p, uint2 q)
{
	return int2(p.x != q.x, p.y != q.y);
}
static inline int3 operator!=(uint3 p, uint3 q)
{
	return int3(p.x != q.x, p.y != q.y, p.z != q.z);
}
static inline int4 operator!=(uint4 p, uint4 q)
{
	return int4(p.x != q.x, p.y != q.y, p.z != q.z, p.w != q.w);
}
static inline int2 operator!=(uint2 p, uint q)
{
	return int2(p.x != q, p.y != q);
}
static inline int3 operator!=(uint3 p, uint q)
{
	return int3(p.x != q, p.y != q, p.z != q);
}
static inline int4 operator!=(uint4 p, uint q)
{
	return int4(p.x != q, p.y != q, p.z != q, p.w != q);
}
static inline int2 operator!=(uint p, uint2 q)
{
	return int2(p != q.x, p != q.y);
}
static inline int3 operator!=(uint p, uint3 q)
{
	return int3(p != q.x, p != q.y, p != q.z);
}
static inline int4 operator!=(uint p, uint4 q)
{
	return int4(p != q.x, p != q.y, p != q.z, p != q.w);
}
static inline short2 operator!=(short2 p, short2 q)
{
	return short2(p.x != q.x, p.y != q.y);
}
static inline short3 operator!=(short3 p, short3 q)
{
	return short3(p.x != q.x, p.y != q.y, p.z != q.z);
}
static inline short4 operator!=(short4 p, short4 q)
{
	return short4(p.x != q.x, p.y != q.y, p.z != q.z, p.w != q.w);
}
static inline short2 operator!=(short2 p, short q)
{
	return short2(p.x != q, p.y != q);
}
static inline short3 operator!=(short3 p, short q)
{
	return short3(p.x != q, p.y != q, p.z != q);
}
static inline short4 operator!=(short4 p, short q)
{
	return short4(p.x != q, p.y != q, p.z != q, p.w != q);
}
static inline short2 operator!=(short p, short2 q)
{
	return short2(p != q.x, p != q.y);
}
static inline short3 operator!=(short p, short3 q)
{
	return short3(p != q.x, p != q.y, p != q.z);
}
static inline short4 operator!=(short p, short4 q)
{
	return short4(p != q.x, p != q.y, p != q.z, p != q.w);
}
static inline short2 operator!=(ushort2 p, ushort2 q)
{
	return short2(p.x != q.x, p.y != q.y);
}
static inline short3 operator!=(ushort3 p, ushort3 q)
{
	return short3(p.x != q.x, p.y != q.y, p.z != q.z);
}
static inline short4 operator!=(ushort4 p, ushort4 q)
{
	return short4(p.x != q.x, p.y != q.y, p.z != q.z, p.w != q.w);
}
static inline short2 operator!=(ushort2 p, ushort q)
{
	return short2(p.x != q, p.y != q);
}
static inline short3 operator!=(ushort3 p, ushort q)
{
	return short3(p.x != q, p.y != q, p.z != q);
}
static inline short4 operator!=(ushort4 p, ushort q)
{
	return short4(p.x != q, p.y != q, p.z != q, p.w != q);
}
static inline short2 operator!=(ushort p, ushort2 q)
{
	return short2(p != q.x, p != q.y);
}
static inline short3 operator!=(ushort p, ushort3 q)
{
	return short3(p != q.x, p != q.y, p != q.z);
}
static inline short4 operator!=(ushort p, ushort4 q)
{
	return short4(p != q.x, p != q.y, p != q.z, p != q.w);
}
static inline long2 operator!=(long2 p, long2 q)
{
	return long2(p.x != q.x, p.y != q.y);
}
static inline long3 operator!=(long3 p, long3 q)
{
	return long3(p.x != q.x, p.y != q.y, p.z != q.z);
}
static inline long4 operator!=(long4 p, long4 q)
{
	return long4(p.x != q.x, p.y != q.y, p.z != q.z, p.w != q.w);
}
static inline long2 operator!=(long2 p, long q)
{
	return long2(p.x != q, p.y != q);
}
static inline long3 operator!=(long3 p, long q)
{
	return long3(p.x != q, p.y != q, p.z != q);
}
static inline long4 operator!=(long4 p, long q)
{
	return long4(p.x != q, p.y != q, p.z != q, p.w != q);
}
static inline long2 operator!=(long p, long2 q)
{
	return long2(p != q.x, p != q.y);
}
static inline long3 operator!=(long p, long3 q)
{
	return long3(p != q.x, p != q.y, p != q.z);
}
static inline long4 operator!=(long p, long4 q)
{
	return long4(p != q.x, p != q.y, p != q.z, p != q.w);
}
static inline long2 operator!=(ulong2 p, ulong2 q)
{
	return long2(p.x != q.x, p.y != q.y);
}
static inline long3 operator!=(ulong3 p, ulong3 q)
{
	return long3(p.x != q.x, p.y != q.y, p.z != q.z);
}
static inline long4 operator!=(ulong4 p, ulong4 q)
{
	return long4(p.x != q.x, p.y != q.y, p.z != q.z, p.w != q.w);
}
static inline long2 operator!=(ulong2 p, ulong q)
{
	return long2(p.x != q, p.y != q);
}
static inline long3 operator!=(ulong3 p, ulong q)
{
	return long3(p.x != q, p.y != q, p.z != q);
}
static inline long4 operator!=(ulong4 p, ulong q)
{
	return long4(p.x != q, p.y != q, p.z != q, p.w != q);
}
static inline long2 operator!=(ulong p, ulong2 q)
{
	return long2(p != q.x, p != q.y);
}
static inline long3 operator!=(ulong p, ulong3 q)
{
	return long3(p != q.x, p != q.y, p != q.z);
}
static inline long4 operator!=(ulong p, ulong4 q)
{
	return long4(p != q.x, p != q.y, p != q.z, p != q.w);
}
static inline int2 operator>(float2 p, float2 q)
{
	return int2(p.x > q.x, p.y > q.y);
}
static inline int3 operator>(float3 p, float3 q)
{
	return int3(p.x > q.x, p.y > q.y, p.z > q.z);
}
static inline int4 operator>(float4 p, float4 q)
{
	return int4(p.x > q.x, p.y > q.y, p.z > q.z, p.w > q.w);
}
static inline int2 operator>(float2 p, float q)
{
	return int2(p.x > q, p.y > q);
}
static inline int3 operator>(float3 p, float q)
{
	return int3(p.x > q, p.y > q, p.z > q);
}
static inline int4 operator>(float4 p, float q)
{
	return int4(p.x > q, p.y > q, p.z > q, p.w > q);
}
static inline int2 operator>(float p, float2 q)
{
	return int2(p > q.x, p > q.y);
}
static inline int3 operator>(float p, float3 q)
{
	return int3(p > q.x, p > q.y, p > q.z);
}
static inline int4 operator>(float p, float4 q)
{
	return int4(p > q.x, p > q.y, p > q.z, p > q.w);
}
static inline long2 operator>(double2 p, double2 q)
{
	return long2(p.x > q.x, p.y > q.y);
}
static inline long3 operator>(double3 p, double3 q)
{
	return long3(p.x > q.x, p.y > q.y, p.z > q.z);
}
static inline long4 operator>(double4 p, double4 q)
{
	return long4(p.x > q.x, p.y > q.y, p.z > q.z, p.w > q.w);
}
static inline long2 operator>(double2 p, double q)
{
	return long2(p.x > q, p.y > q);
}
static inline long3 operator>(double3 p, double q)
{
	return long3(p.x > q, p.y > q, p.z > q);
}
static inline long4 operator>(double4 p, double q)
{
	return long4(p.x > q, p.y > q, p.z > q, p.w > q);
}
static inline long2 operator>(double p, double2 q)
{
	return long2(p > q.x, p > q.y);
}
static inline long3 operator>(double p, double3 q)
{
	return long3(p > q.x, p > q.y, p > q.z);
}
static inline long4 operator>(double p, double4 q)
{
	return long4(p > q.x, p > q.y, p > q.z, p > q.w);
}
static inline int2 operator>(int2 p, int2 q)
{
	return int2(p.x > q.x, p.y > q.y);
}
static inline int3 operator>(int3 p, int3 q)
{
	return int3(p.x > q.x, p.y > q.y, p.z > q.z);
}
static inline int4 operator>(int4 p, int4 q)
{
	return int4(p.x > q.x, p.y > q.y, p.z > q.z, p.w > q.w);
}
static inline int2 operator>(int2 p, int q)
{
	return int2(p.x > q, p.y > q);
}
static inline int3 operator>(int3 p, int q)
{
	return int3(p.x > q, p.y > q, p.z > q);
}
static inline int4 operator>(int4 p, int q)
{
	return int4(p.x > q, p.y > q, p.z > q, p.w > q);
}
static inline int2 operator>(int p, int2 q)
{
	return int2(p > q.x, p > q.y);
}
static inline int3 operator>(int p, int3 q)
{
	return int3(p > q.x, p > q.y, p > q.z);
}
static inline int4 operator>(int p, int4 q)
{
	return int4(p > q.x, p > q.y, p > q.z, p > q.w);
}
static inline int2 operator>(uint2 p, uint2 q)
{
	return int2(p.x > q.x, p.y > q.y);
}
static inline int3 operator>(uint3 p, uint3 q)
{
	return int3(p.x > q.x, p.y > q.y, p.z > q.z);
}
static inline int4 operator>(uint4 p, uint4 q)
{
	return int4(p.x > q.x, p.y > q.y, p.z > q.z, p.w > q.w);
}
static inline int2 operator>(uint2 p, uint q)
{
	return int2(p.x > q, p.y > q);
}
static inline int3 operator>(uint3 p, uint q)
{
	return int3(p.x > q, p.y > q, p.z > q);
}
static inline int4 operator>(uint4 p, uint q)
{
	return int4(p.x > q, p.y > q, p.z > q, p.w > q);
}
static inline int2 operator>(uint p, uint2 q)
{
	return int2(p > q.x, p > q.y);
}
static inline int3 operator>(uint p, uint3 q)
{
	return int3(p > q.x, p > q.y, p > q.z);
}
static inline int4 operator>(uint p, uint4 q)
{
	return int4(p > q.x, p > q.y, p > q.z, p > q.w);
}
static inline short2 operator>(short2 p, short2 q)
{
	return short2(p.x > q.x, p.y > q.y);
}
static inline short3 operator>(short3 p, short3 q)
{
	return short3(p.x > q.x, p.y > q.y, p.z > q.z);
}
static inline short4 operator>(short4 p, short4 q)
{
	return short4(p.x > q.x, p.y > q.y, p.z > q.z, p.w > q.w);
}
static inline short2 operator>(short2 p, short q)
{
	return short2(p.x > q, p.y > q);
}
static inline short3 operator>(short3 p, short q)
{
	return short3(p.x > q, p.y > q, p.z > q);
}
static inline short4 operator>(short4 p, short q)
{
	return short4(p.x > q, p.y > q, p.z > q, p.w > q);
}
static inline short2 operator>(short p, short2 q)
{
	return short2(p > q.x, p > q.y);
}
static inline short3 operator>(short p, short3 q)
{
	return short3(p > q.x, p > q.y, p > q.z);
}
static inline short4 operator>(short p, short4 q)
{
	return short4(p > q.x, p > q.y, p > q.z, p > q.w);
}
static inline short2 operator>(ushort2 p, ushort2 q)
{
	return short2(p.x > q.x, p.y > q.y);
}
static inline short3 operator>(ushort3 p, ushort3 q)
{
	return short3(p.x > q.x, p.y > q.y, p.z > q.z);
}
static inline short4 operator>(ushort4 p, ushort4 q)
{
	return short4(p.x > q.x, p.y > q.y, p.z > q.z, p.w > q.w);
}
static inline short2 operator>(ushort2 p, ushort q)
{
	return short2(p.x > q, p.y > q);
}
static inline short3 operator>(ushort3 p, ushort q)
{
	return short3(p.x > q, p.y > q, p.z > q);
}
static inline short4 operator>(ushort4 p, ushort q)
{
	return short4(p.x > q, p.y > q, p.z > q, p.w > q);
}
static inline short2 operator>(ushort p, ushort2 q)
{
	return short2(p > q.x, p > q.y);
}
static inline short3 operator>(ushort p, ushort3 q)
{
	return short3(p > q.x, p > q.y, p > q.z);
}
static inline short4 operator>(ushort p, ushort4 q)
{
	return short4(p > q.x, p > q.y, p > q.z, p > q.w);
}
static inline long2 operator>(long2 p, long2 q)
{
	return long2(p.x > q.x, p.y > q.y);
}
static inline long3 operator>(long3 p, long3 q)
{
	return long3(p.x > q.x, p.y > q.y, p.z > q.z);
}
static inline long4 operator>(long4 p, long4 q)
{
	return long4(p.x > q.x, p.y > q.y, p.z > q.z, p.w > q.w);
}
static inline long2 operator>(long2 p, long q)
{
	return long2(p.x > q, p.y > q);
}
static inline long3 operator>(long3 p, long q)
{
	return long3(p.x > q, p.y > q, p.z > q);
}
static inline long4 operator>(long4 p, long q)
{
	return long4(p.x > q, p.y > q, p.z > q, p.w > q);
}
static inline long2 operator>(long p, long2 q)
{
	return long2(p > q.x, p > q.y);
}
static inline long3 operator>(long p, long3 q)
{
	return long3(p > q.x, p > q.y, p > q.z);
}
static inline long4 operator>(long p, long4 q)
{
	return long4(p > q.x, p > q.y, p > q.z, p > q.w);
}
static inline long2 operator>(ulong2 p, ulong2 q)
{
	return long2(p.x > q.x, p.y > q.y);
}
static inline long3 operator>(ulong3 p, ulong3 q)
{
	return long3(p.x > q.x, p.y > q.y, p.z > q.z);
}
static inline long4 operator>(ulong4 p, ulong4 q)
{
	return long4(p.x > q.x, p.y > q.y, p.z > q.z, p.w > q.w);
}
static inline long2 operator>(ulong2 p, ulong q)
{
	return long2(p.x > q, p.y > q);
}
static inline long3 operator>(ulong3 p, ulong q)
{
	return long3(p.x > q, p.y > q, p.z > q);
}
static inline long4 operator>(ulong4 p, ulong q)
{
	return long4(p.x > q, p.y > q, p.z > q, p.w > q);
}
static inline long2 operator>(ulong p, ulong2 q)
{
	return long2(p > q.x, p > q.y);
}
static inline long3 operator>(ulong p, ulong3 q)
{
	return long3(p > q.x, p > q.y, p > q.z);
}
static inline long4 operator>(ulong p, ulong4 q)
{
	return long4(p > q.x, p > q.y, p > q.z, p > q.w);
}
static inline int2 operator<(float2 p, float2 q)
{
	return int2(p.x < q.x, p.y < q.y);
}
static inline int3 operator<(float3 p, float3 q)
{
	return int3(p.x < q.x, p.y < q.y, p.z < q.z);
}
static inline int4 operator<(float4 p, float4 q)
{
	return int4(p.x < q.x, p.y < q.y, p.z < q.z, p.w < q.w);
}
static inline int2 operator<(float2 p, float q)
{
	return int2(p.x < q, p.y < q);
}
static inline int3 operator<(float3 p, float q)
{
	return int3(p.x < q, p.y < q, p.z < q);
}
static inline int4 operator<(float4 p, float q)
{
	return int4(p.x < q, p.y < q, p.z < q, p.w < q);
}
static inline int2 operator<(float p, float2 q)
{
	return int2(p < q.x, p < q.y);
}
static inline int3 operator<(float p, float3 q)
{
	return int3(p < q.x, p < q.y, p < q.z);
}
static inline int4 operator<(float p, float4 q)
{
	return int4(p < q.x, p < q.y, p < q.z, p < q.w);
}
static inline long2 operator<(double2 p, double2 q)
{
	return long2(p.x < q.x, p.y < q.y);
}
static inline long3 operator<(double3 p, double3 q)
{
	return long3(p.x < q.x, p.y < q.y, p.z < q.z);
}
static inline long4 operator<(double4 p, double4 q)
{
	return long4(p.x < q.x, p.y < q.y, p.z < q.z, p.w < q.w);
}
static inline long2 operator<(double2 p, double q)
{
	return long2(p.x < q, p.y < q);
}
static inline long3 operator<(double3 p, double q)
{
	return long3(p.x < q, p.y < q, p.z < q);
}
static inline long4 operator<(double4 p, double q)
{
	return long4(p.x < q, p.y < q, p.z < q, p.w < q);
}
static inline long2 operator<(double p, double2 q)
{
	return long2(p < q.x, p < q.y);
}
static inline long3 operator<(double p, double3 q)
{
	return long3(p < q.x, p < q.y, p < q.z);
}
static inline long4 operator<(double p, double4 q)
{
	return long4(p < q.x, p < q.y, p < q.z, p < q.w);
}
static inline int2 operator<(int2 p, int2 q)
{
	return int2(p.x < q.x, p.y < q.y);
}
static inline int3 operator<(int3 p, int3 q)
{
	return int3(p.x < q.x, p.y < q.y, p.z < q.z);
}
static inline int4 operator<(int4 p, int4 q)
{
	return int4(p.x < q.x, p.y < q.y, p.z < q.z, p.w < q.w);
}
static inline int2 operator<(int2 p, int q)
{
	return int2(p.x < q, p.y < q);
}
static inline int3 operator<(int3 p, int q)
{
	return int3(p.x < q, p.y < q, p.z < q);
}
static inline int4 operator<(int4 p, int q)
{
	return int4(p.x < q, p.y < q, p.z < q, p.w < q);
}
static inline int2 operator<(int p, int2 q)
{
	return int2(p < q.x, p < q.y);
}
static inline int3 operator<(int p, int3 q)
{
	return int3(p < q.x, p < q.y, p < q.z);
}
static inline int4 operator<(int p, int4 q)
{
	return int4(p < q.x, p < q.y, p < q.z, p < q.w);
}
static inline int2 operator<(uint2 p, uint2 q)
{
	return int2(p.x < q.x, p.y < q.y);
}
static inline int3 operator<(uint3 p, uint3 q)
{
	return int3(p.x < q.x, p.y < q.y, p.z < q.z);
}
static inline int4 operator<(uint4 p, uint4 q)
{
	return int4(p.x < q.x, p.y < q.y, p.z < q.z, p.w < q.w);
}
static inline int2 operator<(uint2 p, uint q)
{
	return int2(p.x < q, p.y < q);
}
static inline int3 operator<(uint3 p, uint q)
{
	return int3(p.x < q, p.y < q, p.z < q);
}
static inline int4 operator<(uint4 p, uint q)
{
	return int4(p.x < q, p.y < q, p.z < q, p.w < q);
}
static inline int2 operator<(uint p, uint2 q)
{
	return int2(p < q.x, p < q.y);
}
static inline int3 operator<(uint p, uint3 q)
{
	return int3(p < q.x, p < q.y, p < q.z);
}
static inline int4 operator<(uint p, uint4 q)
{
	return int4(p < q.x, p < q.y, p < q.z, p < q.w);
}
static inline short2 operator<(short2 p, short2 q)
{
	return short2(p.x < q.x, p.y < q.y);
}
static inline short3 operator<(short3 p, short3 q)
{
	return short3(p.x < q.x, p.y < q.y, p.z < q.z);
}
static inline short4 operator<(short4 p, short4 q)
{
	return short4(p.x < q.x, p.y < q.y, p.z < q.z, p.w < q.w);
}
static inline short2 operator<(short2 p, short q)
{
	return short2(p.x < q, p.y < q);
}
static inline short3 operator<(short3 p, short q)
{
	return short3(p.x < q, p.y < q, p.z < q);
}
static inline short4 operator<(short4 p, short q)
{
	return short4(p.x < q, p.y < q, p.z < q, p.w < q);
}
static inline short2 operator<(short p, short2 q)
{
	return short2(p < q.x, p < q.y);
}
static inline short3 operator<(short p, short3 q)
{
	return short3(p < q.x, p < q.y, p < q.z);
}
static inline short4 operator<(short p, short4 q)
{
	return short4(p < q.x, p < q.y, p < q.z, p < q.w);
}
static inline short2 operator<(ushort2 p, ushort2 q)
{
	return short2(p.x < q.x, p.y < q.y);
}
static inline short3 operator<(ushort3 p, ushort3 q)
{
	return short3(p.x < q.x, p.y < q.y, p.z < q.z);
}
static inline short4 operator<(ushort4 p, ushort4 q)
{
	return short4(p.x < q.x, p.y < q.y, p.z < q.z, p.w < q.w);
}
static inline short2 operator<(ushort2 p, ushort q)
{
	return short2(p.x < q, p.y < q);
}
static inline short3 operator<(ushort3 p, ushort q)
{
	return short3(p.x < q, p.y < q, p.z < q);
}
static inline short4 operator<(ushort4 p, ushort q)
{
	return short4(p.x < q, p.y < q, p.z < q, p.w < q);
}
static inline short2 operator<(ushort p, ushort2 q)
{
	return short2(p < q.x, p < q.y);
}
static inline short3 operator<(ushort p, ushort3 q)
{
	return short3(p < q.x, p < q.y, p < q.z);
}
static inline short4 operator<(ushort p, ushort4 q)
{
	return short4(p < q.x, p < q.y, p < q.z, p < q.w);
}
static inline long2 operator<(long2 p, long2 q)
{
	return long2(p.x < q.x, p.y < q.y);
}
static inline long3 operator<(long3 p, long3 q)
{
	return long3(p.x < q.x, p.y < q.y, p.z < q.z);
}
static inline long4 operator<(long4 p, long4 q)
{
	return long4(p.x < q.x, p.y < q.y, p.z < q.z, p.w < q.w);
}
static inline long2 operator<(long2 p, long q)
{
	return long2(p.x < q, p.y < q);
}
static inline long3 operator<(long3 p, long q)
{
	return long3(p.x < q, p.y < q, p.z < q);
}
static inline long4 operator<(long4 p, long q)
{
	return long4(p.x < q, p.y < q, p.z < q, p.w < q);
}
static inline long2 operator<(long p, long2 q)
{
	return long2(p < q.x, p < q.y);
}
static inline long3 operator<(long p, long3 q)
{
	return long3(p < q.x, p < q.y, p < q.z);
}
static inline long4 operator<(long p, long4 q)
{
	return long4(p < q.x, p < q.y, p < q.z, p < q.w);
}
static inline long2 operator<(ulong2 p, ulong2 q)
{
	return long2(p.x < q.x, p.y < q.y);
}
static inline long3 operator<(ulong3 p, ulong3 q)
{
	return long3(p.x < q.x, p.y < q.y, p.z < q.z);
}
static inline long4 operator<(ulong4 p, ulong4 q)
{
	return long4(p.x < q.x, p.y < q.y, p.z < q.z, p.w < q.w);
}
static inline long2 operator<(ulong2 p, ulong q)
{
	return long2(p.x < q, p.y < q);
}
static inline long3 operator<(ulong3 p, ulong q)
{
	return long3(p.x < q, p.y < q, p.z < q);
}
static inline long4 operator<(ulong4 p, ulong q)
{
	return long4(p.x < q, p.y < q, p.z < q, p.w < q);
}
static inline long2 operator<(ulong p, ulong2 q)
{
	return long2(p < q.x, p < q.y);
}
static inline long3 operator<(ulong p, ulong3 q)
{
	return long3(p < q.x, p < q.y, p < q.z);
}
static inline long4 operator<(ulong p, ulong4 q)
{
	return long4(p < q.x, p < q.y, p < q.z, p < q.w);
}
static inline int2 operator>=(float2 p, float2 q)
{
	return int2(p.x >= q.x, p.y >= q.y);
}
static inline int3 operator>=(float3 p, float3 q)
{
	return int3(p.x >= q.x, p.y >= q.y, p.z >= q.z);
}
static inline int4 operator>=(float4 p, float4 q)
{
	return int4(p.x >= q.x, p.y >= q.y, p.z >= q.z, p.w >= q.w);
}
static inline int2 operator>=(float2 p, float q)
{
	return int2(p.x >= q, p.y >= q);
}
static inline int3 operator>=(float3 p, float q)
{
	return int3(p.x >= q, p.y >= q, p.z >= q);
}
static inline int4 operator>=(float4 p, float q)
{
	return int4(p.x >= q, p.y >= q, p.z >= q, p.w >= q);
}
static inline int2 operator>=(float p, float2 q)
{
	return int2(p >= q.x, p >= q.y);
}
static inline int3 operator>=(float p, float3 q)
{
	return int3(p >= q.x, p >= q.y, p >= q.z);
}
static inline int4 operator>=(float p, float4 q)
{
	return int4(p >= q.x, p >= q.y, p >= q.z, p >= q.w);
}
static inline long2 operator>=(double2 p, double2 q)
{
	return long2(p.x >= q.x, p.y >= q.y);
}
static inline long3 operator>=(double3 p, double3 q)
{
	return long3(p.x >= q.x, p.y >= q.y, p.z >= q.z);
}
static inline long4 operator>=(double4 p, double4 q)
{
	return long4(p.x >= q.x, p.y >= q.y, p.z >= q.z, p.w >= q.w);
}
static inline long2 operator>=(double2 p, double q)
{
	return long2(p.x >= q, p.y >= q);
}
static inline long3 operator>=(double3 p, double q)
{
	return long3(p.x >= q, p.y >= q, p.z >= q);
}
static inline long4 operator>=(double4 p, double q)
{
	return long4(p.x >= q, p.y >= q, p.z >= q, p.w >= q);
}
static inline long2 operator>=(double p, double2 q)
{
	return long2(p >= q.x, p >= q.y);
}
static inline long3 operator>=(double p, double3 q)
{
	return long3(p >= q.x, p >= q.y, p >= q.z);
}
static inline long4 operator>=(double p, double4 q)
{
	return long4(p >= q.x, p >= q.y, p >= q.z, p >= q.w);
}
static inline int2 operator>=(int2 p, int2 q)
{
	return int2(p.x >= q.x, p.y >= q.y);
}
static inline int3 operator>=(int3 p, int3 q)
{
	return int3(p.x >= q.x, p.y >= q.y, p.z >= q.z);
}
static inline int4 operator>=(int4 p, int4 q)
{
	return int4(p.x >= q.x, p.y >= q.y, p.z >= q.z, p.w >= q.w);
}
static inline int2 operator>=(int2 p, int q)
{
	return int2(p.x >= q, p.y >= q);
}
static inline int3 operator>=(int3 p, int q)
{
	return int3(p.x >= q, p.y >= q, p.z >= q);
}
static inline int4 operator>=(int4 p, int q)
{
	return int4(p.x >= q, p.y >= q, p.z >= q, p.w >= q);
}
static inline int2 operator>=(int p, int2 q)
{
	return int2(p >= q.x, p >= q.y);
}
static inline int3 operator>=(int p, int3 q)
{
	return int3(p >= q.x, p >= q.y, p >= q.z);
}
static inline int4 operator>=(int p, int4 q)
{
	return int4(p >= q.x, p >= q.y, p >= q.z, p >= q.w);
}
static inline int2 operator>=(uint2 p, uint2 q)
{
	return int2(p.x >= q.x, p.y >= q.y);
}
static inline int3 operator>=(uint3 p, uint3 q)
{
	return int3(p.x >= q.x, p.y >= q.y, p.z >= q.z);
}
static inline int4 operator>=(uint4 p, uint4 q)
{
	return int4(p.x >= q.x, p.y >= q.y, p.z >= q.z, p.w >= q.w);
}
static inline int2 operator>=(uint2 p, uint q)
{
	return int2(p.x >= q, p.y >= q);
}
static inline int3 operator>=(uint3 p, uint q)
{
	return int3(p.x >= q, p.y >= q, p.z >= q);
}
static inline int4 operator>=(uint4 p, uint q)
{
	return int4(p.x >= q, p.y >= q, p.z >= q, p.w >= q);
}
static inline int2 operator>=(uint p, uint2 q)
{
	return int2(p >= q.x, p >= q.y);
}
static inline int3 operator>=(uint p, uint3 q)
{
	return int3(p >= q.x, p >= q.y, p >= q.z);
}
static inline int4 operator>=(uint p, uint4 q)
{
	return int4(p >= q.x, p >= q.y, p >= q.z, p >= q.w);
}
static inline short2 operator>=(short2 p, short2 q)
{
	return short2(p.x >= q.x, p.y >= q.y);
}
static inline short3 operator>=(short3 p, short3 q)
{
	return short3(p.x >= q.x, p.y >= q.y, p.z >= q.z);
}
static inline short4 operator>=(short4 p, short4 q)
{
	return short4(p.x >= q.x, p.y >= q.y, p.z >= q.z, p.w >= q.w);
}
static inline short2 operator>=(short2 p, short q)
{
	return short2(p.x >= q, p.y >= q);
}
static inline short3 operator>=(short3 p, short q)
{
	return short3(p.x >= q, p.y >= q, p.z >= q);
}
static inline short4 operator>=(short4 p, short q)
{
	return short4(p.x >= q, p.y >= q, p.z >= q, p.w >= q);
}
static inline short2 operator>=(short p, short2 q)
{
	return short2(p >= q.x, p >= q.y);
}
static inline short3 operator>=(short p, short3 q)
{
	return short3(p >= q.x, p >= q.y, p >= q.z);
}
static inline short4 operator>=(short p, short4 q)
{
	return short4(p >= q.x, p >= q.y, p >= q.z, p >= q.w);
}
static inline short2 operator>=(ushort2 p, ushort2 q)
{
	return short2(p.x >= q.x, p.y >= q.y);
}
static inline short3 operator>=(ushort3 p, ushort3 q)
{
	return short3(p.x >= q.x, p.y >= q.y, p.z >= q.z);
}
static inline short4 operator>=(ushort4 p, ushort4 q)
{
	return short4(p.x >= q.x, p.y >= q.y, p.z >= q.z, p.w >= q.w);
}
static inline short2 operator>=(ushort2 p, ushort q)
{
	return short2(p.x >= q, p.y >= q);
}
static inline short3 operator>=(ushort3 p, ushort q)
{
	return short3(p.x >= q, p.y >= q, p.z >= q);
}
static inline short4 operator>=(ushort4 p, ushort q)
{
	return short4(p.x >= q, p.y >= q, p.z >= q, p.w >= q);
}
static inline short2 operator>=(ushort p, ushort2 q)
{
	return short2(p >= q.x, p >= q.y);
}
static inline short3 operator>=(ushort p, ushort3 q)
{
	return short3(p >= q.x, p >= q.y, p >= q.z);
}
static inline short4 operator>=(ushort p, ushort4 q)
{
	return short4(p >= q.x, p >= q.y, p >= q.z, p >= q.w);
}
static inline long2 operator>=(long2 p, long2 q)
{
	return long2(p.x >= q.x, p.y >= q.y);
}
static inline long3 operator>=(long3 p, long3 q)
{
	return long3(p.x >= q.x, p.y >= q.y, p.z >= q.z);
}
static inline long4 operator>=(long4 p, long4 q)
{
	return long4(p.x >= q.x, p.y >= q.y, p.z >= q.z, p.w >= q.w);
}
static inline long2 operator>=(long2 p, long q)
{
	return long2(p.x >= q, p.y >= q);
}
static inline long3 operator>=(long3 p, long q)
{
	return long3(p.x >= q, p.y >= q, p.z >= q);
}
static inline long4 operator>=(long4 p, long q)
{
	return long4(p.x >= q, p.y >= q, p.z >= q, p.w >= q);
}
static inline long2 operator>=(long p, long2 q)
{
	return long2(p >= q.x, p >= q.y);
}
static inline long3 operator>=(long p, long3 q)
{
	return long3(p >= q.x, p >= q.y, p >= q.z);
}
static inline long4 operator>=(long p, long4 q)
{
	return long4(p >= q.x, p >= q.y, p >= q.z, p >= q.w);
}
static inline long2 operator>=(ulong2 p, ulong2 q)
{
	return long2(p.x >= q.x, p.y >= q.y);
}
static inline long3 operator>=(ulong3 p, ulong3 q)
{
	return long3(p.x >= q.x, p.y >= q.y, p.z >= q.z);
}
static inline long4 operator>=(ulong4 p, ulong4 q)
{
	return long4(p.x >= q.x, p.y >= q.y, p.z >= q.z, p.w >= q.w);
}
static inline long2 operator>=(ulong2 p, ulong q)
{
	return long2(p.x >= q, p.y >= q);
}
static inline long3 operator>=(ulong3 p, ulong q)
{
	return long3(p.x >= q, p.y >= q, p.z >= q);
}
static inline long4 operator>=(ulong4 p, ulong q)
{
	return long4(p.x >= q, p.y >= q, p.z >= q, p.w >= q);
}
static inline long2 operator>=(ulong p, ulong2 q)
{
	return long2(p >= q.x, p >= q.y);
}
static inline long3 operator>=(ulong p, ulong3 q)
{
	return long3(p >= q.x, p >= q.y, p >= q.z);
}
static inline long4 operator>=(ulong p, ulong4 q)
{
	return long4(p >= q.x, p >= q.y, p >= q.z, p >= q.w);
}
static inline int2 operator<=(float2 p, float2 q)
{
	return int2(p.x <= q.x, p.y <= q.y);
}
static inline int3 operator<=(float3 p, float3 q)
{
	return int3(p.x <= q.x, p.y <= q.y, p.z <= q.z);
}
static inline int4 operator<=(float4 p, float4 q)
{
	return int4(p.x <= q.x, p.y <= q.y, p.z <= q.z, p.w <= q.w);
}
static inline int2 operator<=(float2 p, float q)
{
	return int2(p.x <= q, p.y <= q);
}
static inline int3 operator<=(float3 p, float q)
{
	return int3(p.x <= q, p.y <= q, p.z <= q);
}
static inline int4 operator<=(float4 p, float q)
{
	return int4(p.x <= q, p.y <= q, p.z <= q, p.w <= q);
}
static inline int2 operator<=(float p, float2 q)
{
	return int2(p <= q.x, p <= q.y);
}
static inline int3 operator<=(float p, float3 q)
{
	return int3(p <= q.x, p <= q.y, p <= q.z);
}
static inline int4 operator<=(float p, float4 q)
{
	return int4(p <= q.x, p <= q.y, p <= q.z, p <= q.w);
}
static inline long2 operator<=(double2 p, double2 q)
{
	return long2(p.x <= q.x, p.y <= q.y);
}
static inline long3 operator<=(double3 p, double3 q)
{
	return long3(p.x <= q.x, p.y <= q.y, p.z <= q.z);
}
static inline long4 operator<=(double4 p, double4 q)
{
	return long4(p.x <= q.x, p.y <= q.y, p.z <= q.z, p.w <= q.w);
}
static inline long2 operator<=(double2 p, double q)
{
	return long2(p.x <= q, p.y <= q);
}
static inline long3 operator<=(double3 p, double q)
{
	return long3(p.x <= q, p.y <= q, p.z <= q);
}
static inline long4 operator<=(double4 p, double q)
{
	return long4(p.x <= q, p.y <= q, p.z <= q, p.w <= q);
}
static inline long2 operator<=(double p, double2 q)
{
	return long2(p <= q.x, p <= q.y);
}
static inline long3 operator<=(double p, double3 q)
{
	return long3(p <= q.x, p <= q.y, p <= q.z);
}
static inline long4 operator<=(double p, double4 q)
{
	return long4(p <= q.x, p <= q.y, p <= q.z, p <= q.w);
}
static inline int2 operator<=(int2 p, int2 q)
{
	return int2(p.x <= q.x, p.y <= q.y);
}
static inline int3 operator<=(int3 p, int3 q)
{
	return int3(p.x <= q.x, p.y <= q.y, p.z <= q.z);
}
static inline int4 operator<=(int4 p, int4 q)
{
	return int4(p.x <= q.x, p.y <= q.y, p.z <= q.z, p.w <= q.w);
}
static inline int2 operator<=(int2 p, int q)
{
	return int2(p.x <= q, p.y <= q);
}
static inline int3 operator<=(int3 p, int q)
{
	return int3(p.x <= q, p.y <= q, p.z <= q);
}
static inline int4 operator<=(int4 p, int q)
{
	return int4(p.x <= q, p.y <= q, p.z <= q, p.w <= q);
}
static inline int2 operator<=(int p, int2 q)
{
	return int2(p <= q.x, p <= q.y);
}
static inline int3 operator<=(int p, int3 q)
{
	return int3(p <= q.x, p <= q.y, p <= q.z);
}
static inline int4 operator<=(int p, int4 q)
{
	return int4(p <= q.x, p <= q.y, p <= q.z, p <= q.w);
}
static inline int2 operator<=(uint2 p, uint2 q)
{
	return int2(p.x <= q.x, p.y <= q.y);
}
static inline int3 operator<=(uint3 p, uint3 q)
{
	return int3(p.x <= q.x, p.y <= q.y, p.z <= q.z);
}
static inline int4 operator<=(uint4 p, uint4 q)
{
	return int4(p.x <= q.x, p.y <= q.y, p.z <= q.z, p.w <= q.w);
}
static inline int2 operator<=(uint2 p, uint q)
{
	return int2(p.x <= q, p.y <= q);
}
static inline int3 operator<=(uint3 p, uint q)
{
	return int3(p.x <= q, p.y <= q, p.z <= q);
}
static inline int4 operator<=(uint4 p, uint q)
{
	return int4(p.x <= q, p.y <= q, p.z <= q, p.w <= q);
}
static inline int2 operator<=(uint p, uint2 q)
{
	return int2(p <= q.x, p <= q.y);
}
static inline int3 operator<=(uint p, uint3 q)
{
	return int3(p <= q.x, p <= q.y, p <= q.z);
}
static inline int4 operator<=(uint p, uint4 q)
{
	return int4(p <= q.x, p <= q.y, p <= q.z, p <= q.w);
}
static inline short2 operator<=(short2 p, short2 q)
{
	return short2(p.x <= q.x, p.y <= q.y);
}
static inline short3 operator<=(short3 p, short3 q)
{
	return short3(p.x <= q.x, p.y <= q.y, p.z <= q.z);
}
static inline short4 operator<=(short4 p, short4 q)
{
	return short4(p.x <= q.x, p.y <= q.y, p.z <= q.z, p.w <= q.w);
}
static inline short2 operator<=(short2 p, short q)
{
	return short2(p.x <= q, p.y <= q);
}
static inline short3 operator<=(short3 p, short q)
{
	return short3(p.x <= q, p.y <= q, p.z <= q);
}
static inline short4 operator<=(short4 p, short q)
{
	return short4(p.x <= q, p.y <= q, p.z <= q, p.w <= q);
}
static inline short2 operator<=(short p, short2 q)
{
	return short2(p <= q.x, p <= q.y);
}
static inline short3 operator<=(short p, short3 q)
{
	return short3(p <= q.x, p <= q.y, p <= q.z);
}
static inline short4 operator<=(short p, short4 q)
{
	return short4(p <= q.x, p <= q.y, p <= q.z, p <= q.w);
}
static inline short2 operator<=(ushort2 p, ushort2 q)
{
	return short2(p.x <= q.x, p.y <= q.y);
}
static inline short3 operator<=(ushort3 p, ushort3 q)
{
	return short3(p.x <= q.x, p.y <= q.y, p.z <= q.z);
}
static inline short4 operator<=(ushort4 p, ushort4 q)
{
	return short4(p.x <= q.x, p.y <= q.y, p.z <= q.z, p.w <= q.w);
}
static inline short2 operator<=(ushort2 p, ushort q)
{
	return short2(p.x <= q, p.y <= q);
}
static inline short3 operator<=(ushort3 p, ushort q)
{
	return short3(p.x <= q, p.y <= q, p.z <= q);
}
static inline short4 operator<=(ushort4 p, ushort q)
{
	return short4(p.x <= q, p.y <= q, p.z <= q, p.w <= q);
}
static inline short2 operator<=(ushort p, ushort2 q)
{
	return short2(p <= q.x, p <= q.y);
}
static inline short3 operator<=(ushort p, ushort3 q)
{
	return short3(p <= q.x, p <= q.y, p <= q.z);
}
static inline short4 operator<=(ushort p, ushort4 q)
{
	return short4(p <= q.x, p <= q.y, p <= q.z, p <= q.w);
}
static inline long2 operator<=(long2 p, long2 q)
{
	return long2(p.x <= q.x, p.y <= q.y);
}
static inline long3 operator<=(long3 p, long3 q)
{
	return long3(p.x <= q.x, p.y <= q.y, p.z <= q.z);
}
static inline long4 operator<=(long4 p, long4 q)
{
	return long4(p.x <= q.x, p.y <= q.y, p.z <= q.z, p.w <= q.w);
}
static inline long2 operator<=(long2 p, long q)
{
	return long2(p.x <= q, p.y <= q);
}
static inline long3 operator<=(long3 p, long q)
{
	return long3(p.x <= q, p.y <= q, p.z <= q);
}
static inline long4 operator<=(long4 p, long q)
{
	return long4(p.x <= q, p.y <= q, p.z <= q, p.w <= q);
}
static inline long2 operator<=(long p, long2 q)
{
	return long2(p <= q.x, p <= q.y);
}
static inline long3 operator<=(long p, long3 q)
{
	return long3(p <= q.x, p <= q.y, p <= q.z);
}
static inline long4 operator<=(long p, long4 q)
{
	return long4(p <= q.x, p <= q.y, p <= q.z, p <= q.w);
}
static inline long2 operator<=(ulong2 p, ulong2 q)
{
	return long2(p.x <= q.x, p.y <= q.y);
}
static inline long3 operator<=(ulong3 p, ulong3 q)
{
	return long3(p.x <= q.x, p.y <= q.y, p.z <= q.z);
}
static inline long4 operator<=(ulong4 p, ulong4 q)
{
	return long4(p.x <= q.x, p.y <= q.y, p.z <= q.z, p.w <= q.w);
}
static inline long2 operator<=(ulong2 p, ulong q)
{
	return long2(p.x <= q, p.y <= q);
}
static inline long3 operator<=(ulong3 p, ulong q)
{
	return long3(p.x <= q, p.y <= q, p.z <= q);
}
static inline long4 operator<=(ulong4 p, ulong q)
{
	return long4(p.x <= q, p.y <= q, p.z <= q, p.w <= q);
}
static inline long2 operator<=(ulong p, ulong2 q)
{
	return long2(p <= q.x, p <= q.y);
}
static inline long3 operator<=(ulong p, ulong3 q)
{
	return long3(p <= q.x, p <= q.y, p <= q.z);
}
static inline long4 operator<=(ulong p, ulong4 q)
{
	return long4(p <= q.x, p <= q.y, p <= q.z, p <= q.w);
}
static inline int2 operator&&(float2 p, float2 q)
{
	return int2(p.x && q.x, p.y && q.y);
}
static inline int3 operator&&(float3 p, float3 q)
{
	return int3(p.x && q.x, p.y && q.y, p.z && q.z);
}
static inline int4 operator&&(float4 p, float4 q)
{
	return int4(p.x && q.x, p.y && q.y, p.z && q.z, p.w && q.w);
}
static inline int2 operator&&(float2 p, float q)
{
	return int2(p.x && q, p.y && q);
}
static inline int3 operator&&(float3 p, float q)
{
	return int3(p.x && q, p.y && q, p.z && q);
}
static inline int4 operator&&(float4 p, float q)
{
	return int4(p.x && q, p.y && q, p.z && q, p.w && q);
}
static inline int2 operator&&(float p, float2 q)
{
	return int2(p && q.x, p && q.y);
}
static inline int3 operator&&(float p, float3 q)
{
	return int3(p && q.x, p && q.y, p && q.z);
}
static inline int4 operator&&(float p, float4 q)
{
	return int4(p && q.x, p && q.y, p && q.z, p && q.w);
}
static inline long2 operator&&(double2 p, double2 q)
{
	return long2(p.x && q.x, p.y && q.y);
}
static inline long3 operator&&(double3 p, double3 q)
{
	return long3(p.x && q.x, p.y && q.y, p.z && q.z);
}
static inline long4 operator&&(double4 p, double4 q)
{
	return long4(p.x && q.x, p.y && q.y, p.z && q.z, p.w && q.w);
}
static inline long2 operator&&(double2 p, double q)
{
	return long2(p.x && q, p.y && q);
}
static inline long3 operator&&(double3 p, double q)
{
	return long3(p.x && q, p.y && q, p.z && q);
}
static inline long4 operator&&(double4 p, double q)
{
	return long4(p.x && q, p.y && q, p.z && q, p.w && q);
}
static inline long2 operator&&(double p, double2 q)
{
	return long2(p && q.x, p && q.y);
}
static inline long3 operator&&(double p, double3 q)
{
	return long3(p && q.x, p && q.y, p && q.z);
}
static inline long4 operator&&(double p, double4 q)
{
	return long4(p && q.x, p && q.y, p && q.z, p && q.w);
}
static inline int2 operator&&(int2 p, int2 q)
{
	return int2(p.x && q.x, p.y && q.y);
}
static inline int3 operator&&(int3 p, int3 q)
{
	return int3(p.x && q.x, p.y && q.y, p.z && q.z);
}
static inline int4 operator&&(int4 p, int4 q)
{
	return int4(p.x && q.x, p.y && q.y, p.z && q.z, p.w && q.w);
}
static inline int2 operator&&(int2 p, int q)
{
	return int2(p.x && q, p.y && q);
}
static inline int3 operator&&(int3 p, int q)
{
	return int3(p.x && q, p.y && q, p.z && q);
}
static inline int4 operator&&(int4 p, int q)
{
	return int4(p.x && q, p.y && q, p.z && q, p.w && q);
}
static inline int2 operator&&(int p, int2 q)
{
	return int2(p && q.x, p && q.y);
}
static inline int3 operator&&(int p, int3 q)
{
	return int3(p && q.x, p && q.y, p && q.z);
}
static inline int4 operator&&(int p, int4 q)
{
	return int4(p && q.x, p && q.y, p && q.z, p && q.w);
}
static inline int2 operator&&(uint2 p, uint2 q)
{
	return int2(p.x && q.x, p.y && q.y);
}
static inline int3 operator&&(uint3 p, uint3 q)
{
	return int3(p.x && q.x, p.y && q.y, p.z && q.z);
}
static inline int4 operator&&(uint4 p, uint4 q)
{
	return int4(p.x && q.x, p.y && q.y, p.z && q.z, p.w && q.w);
}
static inline int2 operator&&(uint2 p, uint q)
{
	return int2(p.x && q, p.y && q);
}
static inline int3 operator&&(uint3 p, uint q)
{
	return int3(p.x && q, p.y && q, p.z && q);
}
static inline int4 operator&&(uint4 p, uint q)
{
	return int4(p.x && q, p.y && q, p.z && q, p.w && q);
}
static inline int2 operator&&(uint p, uint2 q)
{
	return int2(p && q.x, p && q.y);
}
static inline int3 operator&&(uint p, uint3 q)
{
	return int3(p && q.x, p && q.y, p && q.z);
}
static inline int4 operator&&(uint p, uint4 q)
{
	return int4(p && q.x, p && q.y, p && q.z, p && q.w);
}
static inline short2 operator&&(short2 p, short2 q)
{
	return short2(p.x && q.x, p.y && q.y);
}
static inline short3 operator&&(short3 p, short3 q)
{
	return short3(p.x && q.x, p.y && q.y, p.z && q.z);
}
static inline short4 operator&&(short4 p, short4 q)
{
	return short4(p.x && q.x, p.y && q.y, p.z && q.z, p.w && q.w);
}
static inline short2 operator&&(short2 p, short q)
{
	return short2(p.x && q, p.y && q);
}
static inline short3 operator&&(short3 p, short q)
{
	return short3(p.x && q, p.y && q, p.z && q);
}
static inline short4 operator&&(short4 p, short q)
{
	return short4(p.x && q, p.y && q, p.z && q, p.w && q);
}
static inline short2 operator&&(short p, short2 q)
{
	return short2(p && q.x, p && q.y);
}
static inline short3 operator&&(short p, short3 q)
{
	return short3(p && q.x, p && q.y, p && q.z);
}
static inline short4 operator&&(short p, short4 q)
{
	return short4(p && q.x, p && q.y, p && q.z, p && q.w);
}
static inline short2 operator&&(ushort2 p, ushort2 q)
{
	return short2(p.x && q.x, p.y && q.y);
}
static inline short3 operator&&(ushort3 p, ushort3 q)
{
	return short3(p.x && q.x, p.y && q.y, p.z && q.z);
}
static inline short4 operator&&(ushort4 p, ushort4 q)
{
	return short4(p.x && q.x, p.y && q.y, p.z && q.z, p.w && q.w);
}
static inline short2 operator&&(ushort2 p, ushort q)
{
	return short2(p.x && q, p.y && q);
}
static inline short3 operator&&(ushort3 p, ushort q)
{
	return short3(p.x && q, p.y && q, p.z && q);
}
static inline short4 operator&&(ushort4 p, ushort q)
{
	return short4(p.x && q, p.y && q, p.z && q, p.w && q);
}
static inline short2 operator&&(ushort p, ushort2 q)
{
	return short2(p && q.x, p && q.y);
}
static inline short3 operator&&(ushort p, ushort3 q)
{
	return short3(p && q.x, p && q.y, p && q.z);
}
static inline short4 operator&&(ushort p, ushort4 q)
{
	return short4(p && q.x, p && q.y, p && q.z, p && q.w);
}
static inline long2 operator&&(long2 p, long2 q)
{
	return long2(p.x && q.x, p.y && q.y);
}
static inline long3 operator&&(long3 p, long3 q)
{
	return long3(p.x && q.x, p.y && q.y, p.z && q.z);
}
static inline long4 operator&&(long4 p, long4 q)
{
	return long4(p.x && q.x, p.y && q.y, p.z && q.z, p.w && q.w);
}
static inline long2 operator&&(long2 p, long q)
{
	return long2(p.x && q, p.y && q);
}
static inline long3 operator&&(long3 p, long q)
{
	return long3(p.x && q, p.y && q, p.z && q);
}
static inline long4 operator&&(long4 p, long q)
{
	return long4(p.x && q, p.y && q, p.z && q, p.w && q);
}
static inline long2 operator&&(long p, long2 q)
{
	return long2(p && q.x, p && q.y);
}
static inline long3 operator&&(long p, long3 q)
{
	return long3(p && q.x, p && q.y, p && q.z);
}
static inline long4 operator&&(long p, long4 q)
{
	return long4(p && q.x, p && q.y, p && q.z, p && q.w);
}
static inline long2 operator&&(ulong2 p, ulong2 q)
{
	return long2(p.x && q.x, p.y && q.y);
}
static inline long3 operator&&(ulong3 p, ulong3 q)
{
	return long3(p.x && q.x, p.y && q.y, p.z && q.z);
}
static inline long4 operator&&(ulong4 p, ulong4 q)
{
	return long4(p.x && q.x, p.y && q.y, p.z && q.z, p.w && q.w);
}
static inline long2 operator&&(ulong2 p, ulong q)
{
	return long2(p.x && q, p.y && q);
}
static inline long3 operator&&(ulong3 p, ulong q)
{
	return long3(p.x && q, p.y && q, p.z && q);
}
static inline long4 operator&&(ulong4 p, ulong q)
{
	return long4(p.x && q, p.y && q, p.z && q, p.w && q);
}
static inline long2 operator&&(ulong p, ulong2 q)
{
	return long2(p && q.x, p && q.y);
}
static inline long3 operator&&(ulong p, ulong3 q)
{
	return long3(p && q.x, p && q.y, p && q.z);
}
static inline long4 operator&&(ulong p, ulong4 q)
{
	return long4(p && q.x, p && q.y, p && q.z, p && q.w);
}
static inline int2 operator||(float2 p, float2 q)
{
	return int2(p.x || q.x, p.y || q.y);
}
static inline int3 operator||(float3 p, float3 q)
{
	return int3(p.x || q.x, p.y || q.y, p.z || q.z);
}
static inline int4 operator||(float4 p, float4 q)
{
	return int4(p.x || q.x, p.y || q.y, p.z || q.z, p.w || q.w);
}
static inline int2 operator||(float2 p, float q)
{
	return int2(p.x || q, p.y || q);
}
static inline int3 operator||(float3 p, float q)
{
	return int3(p.x || q, p.y || q, p.z || q);
}
static inline int4 operator||(float4 p, float q)
{
	return int4(p.x || q, p.y || q, p.z || q, p.w || q);
}
static inline int2 operator||(float p, float2 q)
{
	return int2(p || q.x, p || q.y);
}
static inline int3 operator||(float p, float3 q)
{
	return int3(p || q.x, p || q.y, p || q.z);
}
static inline int4 operator||(float p, float4 q)
{
	return int4(p || q.x, p || q.y, p || q.z, p || q.w);
}
static inline long2 operator||(double2 p, double2 q)
{
	return long2(p.x || q.x, p.y || q.y);
}
static inline long3 operator||(double3 p, double3 q)
{
	return long3(p.x || q.x, p.y || q.y, p.z || q.z);
}
static inline long4 operator||(double4 p, double4 q)
{
	return long4(p.x || q.x, p.y || q.y, p.z || q.z, p.w || q.w);
}
static inline long2 operator||(double2 p, double q)
{
	return long2(p.x || q, p.y || q);
}
static inline long3 operator||(double3 p, double q)
{
	return long3(p.x || q, p.y || q, p.z || q);
}
static inline long4 operator||(double4 p, double q)
{
	return long4(p.x || q, p.y || q, p.z || q, p.w || q);
}
static inline long2 operator||(double p, double2 q)
{
	return long2(p || q.x, p || q.y);
}
static inline long3 operator||(double p, double3 q)
{
	return long3(p || q.x, p || q.y, p || q.z);
}
static inline long4 operator||(double p, double4 q)
{
	return long4(p || q.x, p || q.y, p || q.z, p || q.w);
}
static inline int2 operator||(int2 p, int2 q)
{
	return int2(p.x || q.x, p.y || q.y);
}
static inline int3 operator||(int3 p, int3 q)
{
	return int3(p.x || q.x, p.y || q.y, p.z || q.z);
}
static inline int4 operator||(int4 p, int4 q)
{
	return int4(p.x || q.x, p.y || q.y, p.z || q.z, p.w || q.w);
}
static inline int2 operator||(int2 p, int q)
{
	return int2(p.x || q, p.y || q);
}
static inline int3 operator||(int3 p, int q)
{
	return int3(p.x || q, p.y || q, p.z || q);
}
static inline int4 operator||(int4 p, int q)
{
	return int4(p.x || q, p.y || q, p.z || q, p.w || q);
}
static inline int2 operator||(int p, int2 q)
{
	return int2(p || q.x, p || q.y);
}
static inline int3 operator||(int p, int3 q)
{
	return int3(p || q.x, p || q.y, p || q.z);
}
static inline int4 operator||(int p, int4 q)
{
	return int4(p || q.x, p || q.y, p || q.z, p || q.w);
}
static inline int2 operator||(uint2 p, uint2 q)
{
	return int2(p.x || q.x, p.y || q.y);
}
static inline int3 operator||(uint3 p, uint3 q)
{
	return int3(p.x || q.x, p.y || q.y, p.z || q.z);
}
static inline int4 operator||(uint4 p, uint4 q)
{
	return int4(p.x || q.x, p.y || q.y, p.z || q.z, p.w || q.w);
}
static inline int2 operator||(uint2 p, uint q)
{
	return int2(p.x || q, p.y || q);
}
static inline int3 operator||(uint3 p, uint q)
{
	return int3(p.x || q, p.y || q, p.z || q);
}
static inline int4 operator||(uint4 p, uint q)
{
	return int4(p.x || q, p.y || q, p.z || q, p.w || q);
}
static inline int2 operator||(uint p, uint2 q)
{
	return int2(p || q.x, p || q.y);
}
static inline int3 operator||(uint p, uint3 q)
{
	return int3(p || q.x, p || q.y, p || q.z);
}
static inline int4 operator||(uint p, uint4 q)
{
	return int4(p || q.x, p || q.y, p || q.z, p || q.w);
}
static inline short2 operator||(short2 p, short2 q)
{
	return short2(p.x || q.x, p.y || q.y);
}
static inline short3 operator||(short3 p, short3 q)
{
	return short3(p.x || q.x, p.y || q.y, p.z || q.z);
}
static inline short4 operator||(short4 p, short4 q)
{
	return short4(p.x || q.x, p.y || q.y, p.z || q.z, p.w || q.w);
}
static inline short2 operator||(short2 p, short q)
{
	return short2(p.x || q, p.y || q);
}
static inline short3 operator||(short3 p, short q)
{
	return short3(p.x || q, p.y || q, p.z || q);
}
static inline short4 operator||(short4 p, short q)
{
	return short4(p.x || q, p.y || q, p.z || q, p.w || q);
}
static inline short2 operator||(short p, short2 q)
{
	return short2(p || q.x, p || q.y);
}
static inline short3 operator||(short p, short3 q)
{
	return short3(p || q.x, p || q.y, p || q.z);
}
static inline short4 operator||(short p, short4 q)
{
	return short4(p || q.x, p || q.y, p || q.z, p || q.w);
}
static inline short2 operator||(ushort2 p, ushort2 q)
{
	return short2(p.x || q.x, p.y || q.y);
}
static inline short3 operator||(ushort3 p, ushort3 q)
{
	return short3(p.x || q.x, p.y || q.y, p.z || q.z);
}
static inline short4 operator||(ushort4 p, ushort4 q)
{
	return short4(p.x || q.x, p.y || q.y, p.z || q.z, p.w || q.w);
}
static inline short2 operator||(ushort2 p, ushort q)
{
	return short2(p.x || q, p.y || q);
}
static inline short3 operator||(ushort3 p, ushort q)
{
	return short3(p.x || q, p.y || q, p.z || q);
}
static inline short4 operator||(ushort4 p, ushort q)
{
	return short4(p.x || q, p.y || q, p.z || q, p.w || q);
}
static inline short2 operator||(ushort p, ushort2 q)
{
	return short2(p || q.x, p || q.y);
}
static inline short3 operator||(ushort p, ushort3 q)
{
	return short3(p || q.x, p || q.y, p || q.z);
}
static inline short4 operator||(ushort p, ushort4 q)
{
	return short4(p || q.x, p || q.y, p || q.z, p || q.w);
}
static inline long2 operator||(long2 p, long2 q)
{
	return long2(p.x || q.x, p.y || q.y);
}
static inline long3 operator||(long3 p, long3 q)
{
	return long3(p.x || q.x, p.y || q.y, p.z || q.z);
}
static inline long4 operator||(long4 p, long4 q)
{
	return long4(p.x || q.x, p.y || q.y, p.z || q.z, p.w || q.w);
}
static inline long2 operator||(long2 p, long q)
{
	return long2(p.x || q, p.y || q);
}
static inline long3 operator||(long3 p, long q)
{
	return long3(p.x || q, p.y || q, p.z || q);
}
static inline long4 operator||(long4 p, long q)
{
	return long4(p.x || q, p.y || q, p.z || q, p.w || q);
}
static inline long2 operator||(long p, long2 q)
{
	return long2(p || q.x, p || q.y);
}
static inline long3 operator||(long p, long3 q)
{
	return long3(p || q.x, p || q.y, p || q.z);
}
static inline long4 operator||(long p, long4 q)
{
	return long4(p || q.x, p || q.y, p || q.z, p || q.w);
}
static inline long2 operator||(ulong2 p, ulong2 q)
{
	return long2(p.x || q.x, p.y || q.y);
}
static inline long3 operator||(ulong3 p, ulong3 q)
{
	return long3(p.x || q.x, p.y || q.y, p.z || q.z);
}
static inline long4 operator||(ulong4 p, ulong4 q)
{
	return long4(p.x || q.x, p.y || q.y, p.z || q.z, p.w || q.w);
}
static inline long2 operator||(ulong2 p, ulong q)
{
	return long2(p.x || q, p.y || q);
}
static inline long3 operator||(ulong3 p, ulong q)
{
	return long3(p.x || q, p.y || q, p.z || q);
}
static inline long4 operator||(ulong4 p, ulong q)
{
	return long4(p.x || q, p.y || q, p.z || q, p.w || q);
}
static inline long2 operator||(ulong p, ulong2 q)
{
	return long2(p || q.x, p || q.y);
}
static inline long3 operator||(ulong p, ulong3 q)
{
	return long3(p || q.x, p || q.y, p || q.z);
}
static inline long4 operator||(ulong p, ulong4 q)
{
	return long4(p || q.x, p || q.y, p || q.z, p || q.w);
}
static inline float2 convert_float2(float2 p)
{
	return float2((float)p.x, (float)p.y);
}
static inline float3 convert_float3(float3 p)
{
	return float3((float)p.x, (float)p.y, (float)p.z);
}
static inline float4 convert_float4(float4 p)
{
	return float4((float)p.x, (float)p.y, (float)p.z, (float)p.w);
}
static inline float2 convert_float2(double2 p)
{
	return float2((float)p.x, (float)p.y);
}
static inline float3 convert_float3(double3 p)
{
	return float3((float)p.x, (float)p.y, (float)p.z);
}
static inline float4 convert_float4(double4 p)
{
	return float4((float)p.x, (float)p.y, (float)p.z, (float)p.w);
}
static inline float2 convert_float2(int2 p)
{
	return float2((float)p.x, (float)p.y);
}
static inline float3 convert_float3(int3 p)
{
	return float3((float)p.x, (float)p.y, (float)p.z);
}
static inline float4 convert_float4(int4 p)
{
	return float4((float)p.x, (float)p.y, (float)p.z, (float)p.w);
}
static inline float2 convert_float2(uint2 p)
{
	return float2((float)p.x, (float)p.y);
}
static inline float3 convert_float3(uint3 p)
{
	return float3((float)p.x, (float)p.y, (float)p.z);
}
static inline float4 convert_float4(uint4 p)
{
	return float4((float)p.x, (float)p.y, (float)p.z, (float)p.w);
}
static inline float2 convert_float2(short2 p)
{
	return float2((float)p.x, (float)p.y);
}
static inline float3 convert_float3(short3 p)
{
	return float3((float)p.x, (float)p.y, (float)p.z);
}
static inline float4 convert_float4(short4 p)
{
	return float4((float)p.x, (float)p.y, (float)p.z, (float)p.w);
}
static inline float2 convert_float2(ushort2 p)
{
	return float2((float)p.x, (float)p.y);
}
static inline float3 convert_float3(ushort3 p)
{
	return float3((float)p.x, (float)p.y, (float)p.z);
}
static inline float4 convert_float4(ushort4 p)
{
	return float4((float)p.x, (float)p.y, (float)p.z, (float)p.w);
}
static inline float2 convert_float2(long2 p)
{
	return float2((float)p.x, (float)p.y);
}
static inline float3 convert_float3(long3 p)
{
	return float3((float)p.x, (float)p.y, (float)p.z);
}
static inline float4 convert_float4(long4 p)
{
	return float4((float)p.x, (float)p.y, (float)p.z, (float)p.w);
}
static inline float2 convert_float2(ulong2 p)
{
	return float2((float)p.x, (float)p.y);
}
static inline float3 convert_float3(ulong3 p)
{
	return float3((float)p.x, (float)p.y, (float)p.z);
}
static inline float4 convert_float4(ulong4 p)
{
	return float4((float)p.x, (float)p.y, (float)p.z, (float)p.w);
}
static inline double2 convert_double2(float2 p)
{
	return double2((double)p.x, (double)p.y);
}
static inline double3 convert_double3(float3 p)
{
	return double3((double)p.x, (double)p.y, (double)p.z);
}
static inline double4 convert_double4(float4 p)
{
	return double4((double)p.x, (double)p.y, (double)p.z, (double)p.w);
}
static inline double2 convert_double2(double2 p)
{
	return double2((double)p.x, (double)p.y);
}
static inline double3 convert_double3(double3 p)
{
	return double3((double)p.x, (double)p.y, (double)p.z);
}
static inline double4 convert_double4(double4 p)
{
	return double4((double)p.x, (double)p.y, (double)p.z, (double)p.w);
}
static inline double2 convert_double2(int2 p)
{
	return double2((double)p.x, (double)p.y);
}
static inline double3 convert_double3(int3 p)
{
	return double3((double)p.x, (double)p.y, (double)p.z);
}
static inline double4 convert_double4(int4 p)
{
	return double4((double)p.x, (double)p.y, (double)p.z, (double)p.w);
}
static inline double2 convert_double2(uint2 p)
{
	return double2((double)p.x, (double)p.y);
}
static inline double3 convert_double3(uint3 p)
{
	return double3((double)p.x, (double)p.y, (double)p.z);
}
static inline double4 convert_double4(uint4 p)
{
	return double4((double)p.x, (double)p.y, (double)p.z, (double)p.w);
}
static inline double2 convert_double2(short2 p)
{
	return double2((double)p.x, (double)p.y);
}
static inline double3 convert_double3(short3 p)
{
	return double3((double)p.x, (double)p.y, (double)p.z);
}
static inline double4 convert_double4(short4 p)
{
	return double4((double)p.x, (double)p.y, (double)p.z, (double)p.w);
}
static inline double2 convert_double2(ushort2 p)
{
	return double2((double)p.x, (double)p.y);
}
static inline double3 convert_double3(ushort3 p)
{
	return double3((double)p.x, (double)p.y, (double)p.z);
}
static inline double4 convert_double4(ushort4 p)
{
	return double4((double)p.x, (double)p.y, (double)p.z, (double)p.w);
}
static inline double2 convert_double2(long2 p)
{
	return double2((double)p.x, (double)p.y);
}
static inline double3 convert_double3(long3 p)
{
	return double3((double)p.x, (double)p.y, (double)p.z);
}
static inline double4 convert_double4(long4 p)
{
	return double4((double)p.x, (double)p.y, (double)p.z, (double)p.w);
}
static inline double2 convert_double2(ulong2 p)
{
	return double2((double)p.x, (double)p.y);
}
static inline double3 convert_double3(ulong3 p)
{
	return double3((double)p.x, (double)p.y, (double)p.z);
}
static inline double4 convert_double4(ulong4 p)
{
	return double4((double)p.x, (double)p.y, (double)p.z, (double)p.w);
}
static inline int2 convert_int2(float2 p)
{
	return int2((int)p.x, (int)p.y);
}
static inline int3 convert_int3(float3 p)
{
	return int3((int)p.x, (int)p.y, (int)p.z);
}
static inline int4 convert_int4(float4 p)
{
	return int4((int)p.x, (int)p.y, (int)p.z, (int)p.w);
}
static inline int2 convert_int2(double2 p)
{
	return int2((int)p.x, (int)p.y);
}
static inline int3 convert_int3(double3 p)
{
	return int3((int)p.x, (int)p.y, (int)p.z);
}
static inline int4 convert_int4(double4 p)
{
	return int4((int)p.x, (int)p.y, (int)p.z, (int)p.w);
}
static inline int2 convert_int2(int2 p)
{
	return int2((int)p.x, (int)p.y);
}
static inline int3 convert_int3(int3 p)
{
	return int3((int)p.x, (int)p.y, (int)p.z);
}
static inline int4 convert_int4(int4 p)
{
	return int4((int)p.x, (int)p.y, (int)p.z, (int)p.w);
}
static inline int2 convert_int2(uint2 p)
{
	return int2((int)p.x, (int)p.y);
}
static inline int3 convert_int3(uint3 p)
{
	return int3((int)p.x, (int)p.y, (int)p.z);
}
static inline int4 convert_int4(uint4 p)
{
	return int4((int)p.x, (int)p.y, (int)p.z, (int)p.w);
}
static inline int2 convert_int2(short2 p)
{
	return int2((int)p.x, (int)p.y);
}
static inline int3 convert_int3(short3 p)
{
	return int3((int)p.x, (int)p.y, (int)p.z);
}
static inline int4 convert_int4(short4 p)
{
	return int4((int)p.x, (int)p.y, (int)p.z, (int)p.w);
}
static inline int2 convert_int2(ushort2 p)
{
	return int2((int)p.x, (int)p.y);
}
static inline int3 convert_int3(ushort3 p)
{
	return int3((int)p.x, (int)p.y, (int)p.z);
}
static inline int4 convert_int4(ushort4 p)
{
	return int4((int)p.x, (int)p.y, (int)p.z, (int)p.w);
}
static inline int2 convert_int2(long2 p)
{
	return int2((int)p.x, (int)p.y);
}
static inline int3 convert_int3(long3 p)
{
	return int3((int)p.x, (int)p.y, (int)p.z);
}
static inline int4 convert_int4(long4 p)
{
	return int4((int)p.x, (int)p.y, (int)p.z, (int)p.w);
}
static inline int2 convert_int2(ulong2 p)
{
	return int2((int)p.x, (int)p.y);
}
static inline int3 convert_int3(ulong3 p)
{
	return int3((int)p.x, (int)p.y, (int)p.z);
}
static inline int4 convert_int4(ulong4 p)
{
	return int4((int)p.x, (int)p.y, (int)p.z, (int)p.w);
}
static inline uint2 convert_uint2(float2 p)
{
	return uint2((uint) p.x, (uint) p.y);
}
static inline uint3 convert_uint3(float3 p)
{
	return uint3((uint) p.x, (uint) p.y, (uint) p.z);
}
static inline uint4 convert_uint4(float4 p)
{
	return uint4((uint) p.x, (uint) p.y, (uint) p.z, (uint) p.w);
}
static inline uint2 convert_uint2(double2 p)
{
	return uint2((uint) p.x, (uint) p.y);
}
static inline uint3 convert_uint3(double3 p)
{
	return uint3((uint) p.x, (uint) p.y, (uint) p.z);
}
static inline uint4 convert_uint4(double4 p)
{
	return uint4((uint) p.x, (uint) p.y, (uint) p.z, (uint) p.w);
}
static inline uint2 convert_uint2(int2 p)
{
	return uint2((uint) p.x, (uint) p.y);
}
static inline uint3 convert_uint3(int3 p)
{
	return uint3((uint) p.x, (uint) p.y, (uint) p.z);
}
static inline uint4 convert_uint4(int4 p)
{
	return uint4((uint) p.x, (uint) p.y, (uint) p.z, (uint) p.w);
}
static inline uint2 convert_uint2(uint2 p)
{
	return uint2((uint) p.x, (uint) p.y);
}
static inline uint3 convert_uint3(uint3 p)
{
	return uint3((uint) p.x, (uint) p.y, (uint) p.z);
}
static inline uint4 convert_uint4(uint4 p)
{
	return uint4((uint) p.x, (uint) p.y, (uint) p.z, (uint) p.w);
}
static inline uint2 convert_uint2(short2 p)
{
	return uint2((uint) p.x, (uint) p.y);
}
static inline uint3 convert_uint3(short3 p)
{
	return uint3((uint) p.x, (uint) p.y, (uint) p.z);
}
static inline uint4 convert_uint4(short4 p)
{
	return uint4((uint) p.x, (uint) p.y, (uint) p.z, (uint) p.w);
}
static inline uint2 convert_uint2(ushort2 p)
{
	return uint2((uint) p.x, (uint) p.y);
}
static inline uint3 convert_uint3(ushort3 p)
{
	return uint3((uint) p.x, (uint) p.y, (uint) p.z);
}
static inline uint4 convert_uint4(ushort4 p)
{
	return uint4((uint) p.x, (uint) p.y, (uint) p.z, (uint) p.w);
}
static inline uint2 convert_uint2(long2 p)
{
	return uint2((uint) p.x, (uint) p.y);
}
static inline uint3 convert_uint3(long3 p)
{
	return uint3((uint) p.x, (uint) p.y, (uint) p.z);
}
static inline uint4 convert_uint4(long4 p)
{
	return uint4((uint) p.x, (uint) p.y, (uint) p.z, (uint) p.w);
}
static inline uint2 convert_uint2(ulong2 p)
{
	return uint2((uint) p.x, (uint) p.y);
}
static inline uint3 convert_uint3(ulong3 p)
{
	return uint3((uint) p.x, (uint) p.y, (uint) p.z);
}
static inline uint4 convert_uint4(ulong4 p)
{
	return uint4((uint) p.x, (uint) p.y, (uint) p.z, (uint) p.w);
}
static inline short2 convert_short2(float2 p)
{
	return short2((short)p.x, (short)p.y);
}
static inline short3 convert_short3(float3 p)
{
	return short3((short)p.x, (short)p.y, (short)p.z);
}
static inline short4 convert_short4(float4 p)
{
	return short4((short)p.x, (short)p.y, (short)p.z, (short)p.w);
}
static inline short2 convert_short2(double2 p)
{
	return short2((short)p.x, (short)p.y);
}
static inline short3 convert_short3(double3 p)
{
	return short3((short)p.x, (short)p.y, (short)p.z);
}
static inline short4 convert_short4(double4 p)
{
	return short4((short)p.x, (short)p.y, (short)p.z, (short)p.w);
}
static inline short2 convert_short2(int2 p)
{
	return short2((short)p.x, (short)p.y);
}
static inline short3 convert_short3(int3 p)
{
	return short3((short)p.x, (short)p.y, (short)p.z);
}
static inline short4 convert_short4(int4 p)
{
	return short4((short)p.x, (short)p.y, (short)p.z, (short)p.w);
}
static inline short2 convert_short2(uint2 p)
{
	return short2((short)p.x, (short)p.y);
}
static inline short3 convert_short3(uint3 p)
{
	return short3((short)p.x, (short)p.y, (short)p.z);
}
static inline short4 convert_short4(uint4 p)
{
	return short4((short)p.x, (short)p.y, (short)p.z, (short)p.w);
}
static inline short2 convert_short2(short2 p)
{
	return short2((short)p.x, (short)p.y);
}
static inline short3 convert_short3(short3 p)
{
	return short3((short)p.x, (short)p.y, (short)p.z);
}
static inline short4 convert_short4(short4 p)
{
	return short4((short)p.x, (short)p.y, (short)p.z, (short)p.w);
}
static inline short2 convert_short2(ushort2 p)
{
	return short2((short)p.x, (short)p.y);
}
static inline short3 convert_short3(ushort3 p)
{
	return short3((short)p.x, (short)p.y, (short)p.z);
}
static inline short4 convert_short4(ushort4 p)
{
	return short4((short)p.x, (short)p.y, (short)p.z, (short)p.w);
}
static inline short2 convert_short2(long2 p)
{
	return short2((short)p.x, (short)p.y);
}
static inline short3 convert_short3(long3 p)
{
	return short3((short)p.x, (short)p.y, (short)p.z);
}
static inline short4 convert_short4(long4 p)
{
	return short4((short)p.x, (short)p.y, (short)p.z, (short)p.w);
}
static inline short2 convert_short2(ulong2 p)
{
	return short2((short)p.x, (short)p.y);
}
static inline short3 convert_short3(ulong3 p)
{
	return short3((short)p.x, (short)p.y, (short)p.z);
}
static inline short4 convert_short4(ulong4 p)
{
	return short4((short)p.x, (short)p.y, (short)p.z, (short)p.w);
}
static inline ushort2 convert_ushort2(float2 p)
{
	return ushort2((ushort) p.x, (ushort) p.y);
}
static inline ushort3 convert_ushort3(float3 p)
{
	return ushort3((ushort) p.x, (ushort) p.y, (ushort) p.z);
}
static inline ushort4 convert_ushort4(float4 p)
{
	return ushort4((ushort) p.x, (ushort) p.y, (ushort) p.z, (ushort) p.w);
}
static inline ushort2 convert_ushort2(double2 p)
{
	return ushort2((ushort) p.x, (ushort) p.y);
}
static inline ushort3 convert_ushort3(double3 p)
{
	return ushort3((ushort) p.x, (ushort) p.y, (ushort) p.z);
}
static inline ushort4 convert_ushort4(double4 p)
{
	return ushort4((ushort) p.x, (ushort) p.y, (ushort) p.z, (ushort) p.w);
}
static inline ushort2 convert_ushort2(int2 p)
{
	return ushort2((ushort) p.x, (ushort) p.y);
}
static inline ushort3 convert_ushort3(int3 p)
{
	return ushort3((ushort) p.x, (ushort) p.y, (ushort) p.z);
}
static inline ushort4 convert_ushort4(int4 p)
{
	return ushort4((ushort) p.x, (ushort) p.y, (ushort) p.z, (ushort) p.w);
}
static inline ushort2 convert_ushort2(uint2 p)
{
	return ushort2((ushort) p.x, (ushort) p.y);
}
static inline ushort3 convert_ushort3(uint3 p)
{
	return ushort3((ushort) p.x, (ushort) p.y, (ushort) p.z);
}
static inline ushort4 convert_ushort4(uint4 p)
{
	return ushort4((ushort) p.x, (ushort) p.y, (ushort) p.z, (ushort) p.w);
}
static inline ushort2 convert_ushort2(short2 p)
{
	return ushort2((ushort) p.x, (ushort) p.y);
}
static inline ushort3 convert_ushort3(short3 p)
{
	return ushort3((ushort) p.x, (ushort) p.y, (ushort) p.z);
}
static inline ushort4 convert_ushort4(short4 p)
{
	return ushort4((ushort) p.x, (ushort) p.y, (ushort) p.z, (ushort) p.w);
}
static inline ushort2 convert_ushort2(ushort2 p)
{
	return ushort2((ushort) p.x, (ushort) p.y);
}
static inline ushort3 convert_ushort3(ushort3 p)
{
	return ushort3((ushort) p.x, (ushort) p.y, (ushort) p.z);
}
static inline ushort4 convert_ushort4(ushort4 p)
{
	return ushort4((ushort) p.x, (ushort) p.y, (ushort) p.z, (ushort) p.w);
}
static inline ushort2 convert_ushort2(long2 p)
{
	return ushort2((ushort) p.x, (ushort) p.y);
}
static inline ushort3 convert_ushort3(long3 p)
{
	return ushort3((ushort) p.x, (ushort) p.y, (ushort) p.z);
}
static inline ushort4 convert_ushort4(long4 p)
{
	return ushort4((ushort) p.x, (ushort) p.y, (ushort) p.z, (ushort) p.w);
}
static inline ushort2 convert_ushort2(ulong2 p)
{
	return ushort2((ushort) p.x, (ushort) p.y);
}
static inline ushort3 convert_ushort3(ulong3 p)
{
	return ushort3((ushort) p.x, (ushort) p.y, (ushort) p.z);
}
static inline ushort4 convert_ushort4(ulong4 p)
{
	return ushort4((ushort) p.x, (ushort) p.y, (ushort) p.z, (ushort) p.w);
}
static inline long2 convert_long2(float2 p)
{
	return long2((long)p.x, (long)p.y);
}
static inline long3 convert_long3(float3 p)
{
	return long3((long)p.x, (long)p.y, (long)p.z);
}
static inline long4 convert_long4(float4 p)
{
	return long4((long)p.x, (long)p.y, (long)p.z, (long)p.w);
}
static inline long2 convert_long2(double2 p)
{
	return long2((long)p.x, (long)p.y);
}
static inline long3 convert_long3(double3 p)
{
	return long3((long)p.x, (long)p.y, (long)p.z);
}
static inline long4 convert_long4(double4 p)
{
	return long4((long)p.x, (long)p.y, (long)p.z, (long)p.w);
}
static inline long2 convert_long2(int2 p)
{
	return long2((long)p.x, (long)p.y);
}
static inline long3 convert_long3(int3 p)
{
	return long3((long)p.x, (long)p.y, (long)p.z);
}
static inline long4 convert_long4(int4 p)
{
	return long4((long)p.x, (long)p.y, (long)p.z, (long)p.w);
}
static inline long2 convert_long2(uint2 p)
{
	return long2((long)p.x, (long)p.y);
}
static inline long3 convert_long3(uint3 p)
{
	return long3((long)p.x, (long)p.y, (long)p.z);
}
static inline long4 convert_long4(uint4 p)
{
	return long4((long)p.x, (long)p.y, (long)p.z, (long)p.w);
}
static inline long2 convert_long2(short2 p)
{
	return long2((long)p.x, (long)p.y);
}
static inline long3 convert_long3(short3 p)
{
	return long3((long)p.x, (long)p.y, (long)p.z);
}
static inline long4 convert_long4(short4 p)
{
	return long4((long)p.x, (long)p.y, (long)p.z, (long)p.w);
}
static inline long2 convert_long2(ushort2 p)
{
	return long2((long)p.x, (long)p.y);
}
static inline long3 convert_long3(ushort3 p)
{
	return long3((long)p.x, (long)p.y, (long)p.z);
}
static inline long4 convert_long4(ushort4 p)
{
	return long4((long)p.x, (long)p.y, (long)p.z, (long)p.w);
}
static inline long2 convert_long2(long2 p)
{
	return long2((long)p.x, (long)p.y);
}
static inline long3 convert_long3(long3 p)
{
	return long3((long)p.x, (long)p.y, (long)p.z);
}
static inline long4 convert_long4(long4 p)
{
	return long4((long)p.x, (long)p.y, (long)p.z, (long)p.w);
}
static inline long2 convert_long2(ulong2 p)
{
	return long2((long)p.x, (long)p.y);
}
static inline long3 convert_long3(ulong3 p)
{
	return long3((long)p.x, (long)p.y, (long)p.z);
}
static inline long4 convert_long4(ulong4 p)
{
	return long4((long)p.x, (long)p.y, (long)p.z, (long)p.w);
}
static inline ulong2 convert_ulong2(float2 p)
{
	return ulong2((ulong) p.x, (ulong) p.y);
}
static inline ulong3 convert_ulong3(float3 p)
{
	return ulong3((ulong) p.x, (ulong) p.y, (ulong) p.z);
}
static inline ulong4 convert_ulong4(float4 p)
{
	return ulong4((ulong) p.x, (ulong) p.y, (ulong) p.z, (ulong) p.w);
}
static inline ulong2 convert_ulong2(double2 p)
{
	return ulong2((ulong) p.x, (ulong) p.y);
}
static inline ulong3 convert_ulong3(double3 p)
{
	return ulong3((ulong) p.x, (ulong) p.y, (ulong) p.z);
}
static inline ulong4 convert_ulong4(double4 p)
{
	return ulong4((ulong) p.x, (ulong) p.y, (ulong) p.z, (ulong) p.w);
}
static inline ulong2 convert_ulong2(int2 p)
{
	return ulong2((ulong) p.x, (ulong) p.y);
}
static inline ulong3 convert_ulong3(int3 p)
{
	return ulong3((ulong) p.x, (ulong) p.y, (ulong) p.z);
}
static inline ulong4 convert_ulong4(int4 p)
{
	return ulong4((ulong) p.x, (ulong) p.y, (ulong) p.z, (ulong) p.w);
}
static inline ulong2 convert_ulong2(uint2 p)
{
	return ulong2((ulong) p.x, (ulong) p.y);
}
static inline ulong3 convert_ulong3(uint3 p)
{
	return ulong3((ulong) p.x, (ulong) p.y, (ulong) p.z);
}
static inline ulong4 convert_ulong4(uint4 p)
{
	return ulong4((ulong) p.x, (ulong) p.y, (ulong) p.z, (ulong) p.w);
}
static inline ulong2 convert_ulong2(short2 p)
{
	return ulong2((ulong) p.x, (ulong) p.y);
}
static inline ulong3 convert_ulong3(short3 p)
{
	return ulong3((ulong) p.x, (ulong) p.y, (ulong) p.z);
}
static inline ulong4 convert_ulong4(short4 p)
{
	return ulong4((ulong) p.x, (ulong) p.y, (ulong) p.z, (ulong) p.w);
}
static inline ulong2 convert_ulong2(ushort2 p)
{
	return ulong2((ulong) p.x, (ulong) p.y);
}
static inline ulong3 convert_ulong3(ushort3 p)
{
	return ulong3((ulong) p.x, (ulong) p.y, (ulong) p.z);
}
static inline ulong4 convert_ulong4(ushort4 p)
{
	return ulong4((ulong) p.x, (ulong) p.y, (ulong) p.z, (ulong) p.w);
}
static inline ulong2 convert_ulong2(long2 p)
{
	return ulong2((ulong) p.x, (ulong) p.y);
}
static inline ulong3 convert_ulong3(long3 p)
{
	return ulong3((ulong) p.x, (ulong) p.y, (ulong) p.z);
}
static inline ulong4 convert_ulong4(long4 p)
{
	return ulong4((ulong) p.x, (ulong) p.y, (ulong) p.z, (ulong) p.w);
}
static inline ulong2 convert_ulong2(ulong2 p)
{
	return ulong2((ulong) p.x, (ulong) p.y);
}
static inline ulong3 convert_ulong3(ulong3 p)
{
	return ulong3((ulong) p.x, (ulong) p.y, (ulong) p.z);
}
static inline ulong4 convert_ulong4(ulong4 p)
{
	return ulong4((ulong) p.x, (ulong) p.y, (ulong) p.z, (ulong) p.w);
}
static inline float2 as_float2(float2 inp)
{
	float2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline int2 as_int2(float2 inp)
{
	int2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline uint2 as_uint2(float2 inp)
{
	uint2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline double2 as_double2(double2 inp)
{
	double2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline long2 as_long2(double2 inp)
{
	long2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline ulong2 as_ulong2(double2 inp)
{
	ulong2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline float2 as_float2(int2 inp)
{
	float2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline int2 as_int2(int2 inp)
{
	int2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline uint2 as_uint2(int2 inp)
{
	uint2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline float2 as_float2(uint2 inp)
{
	float2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline int2 as_int2(uint2 inp)
{
	int2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline uint2 as_uint2(uint2 inp)
{
	uint2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline short2 as_short2(short2 inp)
{
	short2 val1;
	memcpy(&val1, &inp, 4);
	return val1;
}
static inline ushort2 as_ushort2(short2 inp)
{
	ushort2 val1;
	memcpy(&val1, &inp, 4);
	return val1;
}
static inline short2 as_short2(ushort2 inp)
{
	short2 val1;
	memcpy(&val1, &inp, 4);
	return val1;
}
static inline ushort2 as_ushort2(ushort2 inp)
{
	ushort2 val1;
	memcpy(&val1, &inp, 4);
	return val1;
}
static inline double2 as_double2(long2 inp)
{
	double2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline long2 as_long2(long2 inp)
{
	long2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline ulong2 as_ulong2(long2 inp)
{
	ulong2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline double2 as_double2(ulong2 inp)
{
	double2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline long2 as_long2(ulong2 inp)
{
	long2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline ulong2 as_ulong2(ulong2 inp)
{
	ulong2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline short4 as_short4(float2 inp)
{
	short4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline ushort4 as_ushort4(float2 inp)
{
	ushort4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline float4 as_float4(double2 inp)
{
	float4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline int4 as_int4(double2 inp)
{
	int4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline uint4 as_uint4(double2 inp)
{
	uint4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline short4 as_short4(int2 inp)
{
	short4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline ushort4 as_ushort4(int2 inp)
{
	ushort4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline short4 as_short4(uint2 inp)
{
	short4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline ushort4 as_ushort4(uint2 inp)
{
	ushort4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline float4 as_float4(long2 inp)
{
	float4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline int4 as_int4(long2 inp)
{
	int4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline uint4 as_uint4(long2 inp)
{
	uint4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline float4 as_float4(ulong2 inp)
{
	float4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline int4 as_int4(ulong2 inp)
{
	int4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline uint4 as_uint4(ulong2 inp)
{
	uint4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline float3 as_float3(float3 inp)
{
	float3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline int3 as_int3(float3 inp)
{
	int3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline uint3 as_uint3(float3 inp)
{
	uint3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline double3 as_double3(double3 inp)
{
	double3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline long3 as_long3(double3 inp)
{
	long3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline ulong3 as_ulong3(double3 inp)
{
	ulong3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline float3 as_float3(int3 inp)
{
	float3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline int3 as_int3(int3 inp)
{
	int3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline uint3 as_uint3(int3 inp)
{
	uint3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline float3 as_float3(uint3 inp)
{
	float3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline int3 as_int3(uint3 inp)
{
	int3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline uint3 as_uint3(uint3 inp)
{
	uint3 val1;
	memcpy(&val1, &inp, 12);
	return val1;
}
static inline short3 as_short3(short3 inp)
{
	short3 val1;
	memcpy(&val1, &inp, 6);
	return val1;
}
static inline ushort3 as_ushort3(short3 inp)
{
	ushort3 val1;
	memcpy(&val1, &inp, 6);
	return val1;
}
static inline short3 as_short3(ushort3 inp)
{
	short3 val1;
	memcpy(&val1, &inp, 6);
	return val1;
}
static inline ushort3 as_ushort3(ushort3 inp)
{
	ushort3 val1;
	memcpy(&val1, &inp, 6);
	return val1;
}
static inline double3 as_double3(long3 inp)
{
	double3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline long3 as_long3(long3 inp)
{
	long3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline ulong3 as_ulong3(long3 inp)
{
	ulong3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline double3 as_double3(ulong3 inp)
{
	double3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline long3 as_long3(ulong3 inp)
{
	long3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline ulong3 as_ulong3(ulong3 inp)
{
	ulong3 val1;
	memcpy(&val1, &inp, 24);
	return val1;
}
static inline double2 as_double2(float4 inp)
{
	double2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline long2 as_long2(float4 inp)
{
	long2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline ulong2 as_ulong2(float4 inp)
{
	ulong2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline double2 as_double2(int4 inp)
{
	double2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline long2 as_long2(int4 inp)
{
	long2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline ulong2 as_ulong2(int4 inp)
{
	ulong2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline double2 as_double2(uint4 inp)
{
	double2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline long2 as_long2(uint4 inp)
{
	long2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline ulong2 as_ulong2(uint4 inp)
{
	ulong2 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline float2 as_float2(short4 inp)
{
	float2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline int2 as_int2(short4 inp)
{
	int2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline uint2 as_uint2(short4 inp)
{
	uint2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline float2 as_float2(ushort4 inp)
{
	float2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline int2 as_int2(ushort4 inp)
{
	int2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline uint2 as_uint2(ushort4 inp)
{
	uint2 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline float4 as_float4(float4 inp)
{
	float4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline int4 as_int4(float4 inp)
{
	int4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline uint4 as_uint4(float4 inp)
{
	uint4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline double4 as_double4(double4 inp)
{
	double4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}
static inline long4 as_long4(double4 inp)
{
	long4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}
static inline ulong4 as_ulong4(double4 inp)
{
	ulong4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}
static inline float4 as_float4(int4 inp)
{
	float4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline int4 as_int4(int4 inp)
{
	int4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline uint4 as_uint4(int4 inp)
{
	uint4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline float4 as_float4(uint4 inp)
{
	float4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline int4 as_int4(uint4 inp)
{
	int4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline uint4 as_uint4(uint4 inp)
{
	uint4 val1;
	memcpy(&val1, &inp, 16);
	return val1;
}
static inline short4 as_short4(short4 inp)
{
	short4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline ushort4 as_ushort4(short4 inp)
{
	ushort4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline short4 as_short4(ushort4 inp)
{
	short4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline ushort4 as_ushort4(ushort4 inp)
{
	ushort4 val1;
	memcpy(&val1, &inp, 8);
	return val1;
}
static inline double4 as_double4(long4 inp)
{
	double4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}
static inline long4 as_long4(long4 inp)
{
	long4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}
static inline ulong4 as_ulong4(long4 inp)
{
	ulong4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}
static inline double4 as_double4(ulong4 inp)
{
	double4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}
static inline long4 as_long4(ulong4 inp)
{
	long4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}
static inline ulong4 as_ulong4(ulong4 inp)
{
	ulong4 val1;
	memcpy(&val1, &inp, 32);
	return val1;
}

#define xx xx()
#define xy xy()
#define xz xz()
#define xw xw()
#define yx yx()
#define yy yy()
#define yz yz()
#define yw yw()
#define zx zx()
#define zy zy()
#define zz zz()
#define zw zw()
#define wx wx()
#define wy wy()
#define wz wz()
#define ww ww()
#define xxx xxx()
#define xxy xxy()
#define xxz xxz()
#define xxw xxw()
#define xyx xyx()
#define xyy xyy()
#define xyz xyz()
#define xyw xyw()
#define xzx xzx()
#define xzy xzy()
#define xzz xzz()
#define xzw xzw()
#define xwx xwx()
#define xwy xwy()
#define xwz xwz()
#define xww xww()
#define yxx yxx()
#define yxy yxy()
#define yxz yxz()
#define yxw yxw()
#define yyx yyx()
#define yyy yyy()
#define yyz yyz()
#define yyw yyw()
#define yzx yzx()
#define yzy yzy()
#define yzz yzz()
#define yzw yzw()
#define ywx ywx()
#define ywy ywy()
#define ywz ywz()
#define yww yww()
#define zxx zxx()
#define zxy zxy()
#define zxz zxz()
#define zxw zxw()
#define zyx zyx()
#define zyy zyy()
#define zyz zyz()
#define zyw zyw()
#define zzx zzx()
#define zzy zzy()
#define zzz zzz()
#define zzw zzw()
#define zwx zwx()
#define zwy zwy()
#define zwz zwz()
#define zww zww()
#define wxx wxx()
#define wxy wxy()
#define wxz wxz()
#define wxw wxw()
#define wyx wyx()
#define wyy wyy()
#define wyz wyz()
#define wyw wyw()
#define wzx wzx()
#define wzy wzy()
#define wzz wzz()
#define wzw wzw()
#define wwx wwx()
#define wwy wwy()
#define wwz wwz()
#define www www()
#define xxxx xxxx()
#define xxxy xxxy()
#define xxxz xxxz()
#define xxxw xxxw()
#define xxyx xxyx()
#define xxyy xxyy()
#define xxyz xxyz()
#define xxyw xxyw()
#define xxzx xxzx()
#define xxzy xxzy()
#define xxzz xxzz()
#define xxzw xxzw()
#define xxwx xxwx()
#define xxwy xxwy()
#define xxwz xxwz()
#define xxww xxww()
#define xyxx xyxx()
#define xyxy xyxy()
#define xyxz xyxz()
#define xyxw xyxw()
#define xyyx xyyx()
#define xyyy xyyy()
#define xyyz xyyz()
#define xyyw xyyw()
#define xyzx xyzx()
#define xyzy xyzy()
#define xyzz xyzz()
#define xyzw xyzw()
#define xywx xywx()
#define xywy xywy()
#define xywz xywz()
#define xyww xyww()
#define xzxx xzxx()
#define xzxy xzxy()
#define xzxz xzxz()
#define xzxw xzxw()
#define xzyx xzyx()
#define xzyy xzyy()
#define xzyz xzyz()
#define xzyw xzyw()
#define xzzx xzzx()
#define xzzy xzzy()
#define xzzz xzzz()
#define xzzw xzzw()
#define xzwx xzwx()
#define xzwy xzwy()
#define xzwz xzwz()
#define xzww xzww()
#define xwxx xwxx()
#define xwxy xwxy()
#define xwxz xwxz()
#define xwxw xwxw()
#define xwyx xwyx()
#define xwyy xwyy()
#define xwyz xwyz()
#define xwyw xwyw()
#define xwzx xwzx()
#define xwzy xwzy()
#define xwzz xwzz()
#define xwzw xwzw()
#define xwwx xwwx()
#define xwwy xwwy()
#define xwwz xwwz()
#define xwww xwww()
#define yxxx yxxx()
#define yxxy yxxy()
#define yxxz yxxz()
#define yxxw yxxw()
#define yxyx yxyx()
#define yxyy yxyy()
#define yxyz yxyz()
#define yxyw yxyw()
#define yxzx yxzx()
#define yxzy yxzy()
#define yxzz yxzz()
#define yxzw yxzw()
#define yxwx yxwx()
#define yxwy yxwy()
#define yxwz yxwz()
#define yxww yxww()
#define yyxx yyxx()
#define yyxy yyxy()
#define yyxz yyxz()
#define yyxw yyxw()
#define yyyx yyyx()
#define yyyy yyyy()
#define yyyz yyyz()
#define yyyw yyyw()
#define yyzx yyzx()
#define yyzy yyzy()
#define yyzz yyzz()
#define yyzw yyzw()
#define yywx yywx()
#define yywy yywy()
#define yywz yywz()
#define yyww yyww()
#define yzxx yzxx()
#define yzxy yzxy()
#define yzxz yzxz()
#define yzxw yzxw()
#define yzyx yzyx()
#define yzyy yzyy()
#define yzyz yzyz()
#define yzyw yzyw()
#define yzzx yzzx()
#define yzzy yzzy()
#define yzzz yzzz()
#define yzzw yzzw()
#define yzwx yzwx()
#define yzwy yzwy()
#define yzwz yzwz()
#define yzww yzww()
#define ywxx ywxx()
#define ywxy ywxy()
#define ywxz ywxz()
#define ywxw ywxw()
#define ywyx ywyx()
#define ywyy ywyy()
#define ywyz ywyz()
#define ywyw ywyw()
#define ywzx ywzx()
#define ywzy ywzy()
#define ywzz ywzz()
#define ywzw ywzw()
#define ywwx ywwx()
#define ywwy ywwy()
#define ywwz ywwz()
#define ywww ywww()
#define zxxx zxxx()
#define zxxy zxxy()
#define zxxz zxxz()
#define zxxw zxxw()
#define zxyx zxyx()
#define zxyy zxyy()
#define zxyz zxyz()
#define zxyw zxyw()
#define zxzx zxzx()
#define zxzy zxzy()
#define zxzz zxzz()
#define zxzw zxzw()
#define zxwx zxwx()
#define zxwy zxwy()
#define zxwz zxwz()
#define zxww zxww()
#define zyxx zyxx()
#define zyxy zyxy()
#define zyxz zyxz()
#define zyxw zyxw()
#define zyyx zyyx()
#define zyyy zyyy()
#define zyyz zyyz()
#define zyyw zyyw()
#define zyzx zyzx()
#define zyzy zyzy()
#define zyzz zyzz()
#define zyzw zyzw()
#define zywx zywx()
#define zywy zywy()
#define zywz zywz()
#define zyww zyww()
#define zzxx zzxx()
#define zzxy zzxy()
#define zzxz zzxz()
#define zzxw zzxw()
#define zzyx zzyx()
#define zzyy zzyy()
#define zzyz zzyz()
#define zzyw zzyw()
#define zzzx zzzx()
#define zzzy zzzy()
#define zzzz zzzz()
#define zzzw zzzw()
#define zzwx zzwx()
#define zzwy zzwy()
#define zzwz zzwz()
#define zzww zzww()
#define zwxx zwxx()
#define zwxy zwxy()
#define zwxz zwxz()
#define zwxw zwxw()
#define zwyx zwyx()
#define zwyy zwyy()
#define zwyz zwyz()
#define zwyw zwyw()
#define zwzx zwzx()
#define zwzy zwzy()
#define zwzz zwzz()
#define zwzw zwzw()
#define zwwx zwwx()
#define zwwy zwwy()
#define zwwz zwwz()
#define zwww zwww()
#define wxxx wxxx()
#define wxxy wxxy()
#define wxxz wxxz()
#define wxxw wxxw()
#define wxyx wxyx()
#define wxyy wxyy()
#define wxyz wxyz()
#define wxyw wxyw()
#define wxzx wxzx()
#define wxzy wxzy()
#define wxzz wxzz()
#define wxzw wxzw()
#define wxwx wxwx()
#define wxwy wxwy()
#define wxwz wxwz()
#define wxww wxww()
#define wyxx wyxx()
#define wyxy wyxy()
#define wyxz wyxz()
#define wyxw wyxw()
#define wyyx wyyx()
#define wyyy wyyy()
#define wyyz wyyz()
#define wyyw wyyw()
#define wyzx wyzx()
#define wyzy wyzy()
#define wyzz wyzz()
#define wyzw wyzw()
#define wywx wywx()
#define wywy wywy()
#define wywz wywz()
#define wyww wyww()
#define wzxx wzxx()
#define wzxy wzxy()
#define wzxz wzxz()
#define wzxw wzxw()
#define wzyx wzyx()
#define wzyy wzyy()
#define wzyz wzyz()
#define wzyw wzyw()
#define wzzx wzzx()
#define wzzy wzzy()
#define wzzz wzzz()
#define wzzw wzzw()
#define wzwx wzwx()
#define wzwy wzwy()
#define wzwz wzwz()
#define wzww wzww()
#define wwxx wwxx()
#define wwxy wwxy()
#define wwxz wwxz()
#define wwxw wwxw()
#define wwyx wwyx()
#define wwyy wwyy()
#define wwyz wwyz()
#define wwyw wwyw()
#define wwzx wwzx()
#define wwzy wwzy()
#define wwzz wwzz()
#define wwzw wwzw()
#define wwwx wwwx()
#define wwwy wwwy()
#define wwwz wwwz()
#define wwww wwww()
