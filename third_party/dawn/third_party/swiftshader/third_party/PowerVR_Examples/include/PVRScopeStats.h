/*!***********************************************************************

 @file           PVRScopeStats.h
 @copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved.
 @brief          PVRScopeStats header file. @copybrief ScopeStats
 
**************************************************************************/

/*! @mainpage PVRScope
 
 @section overview Library Overview
*****************************
PVRScope is a utility library which has two functionalities:
 \li @ref ScopeStats is used to access the hardware performance counters in
     PowerVR hardware via a driver library called PVRScopeServices.
 \li @ref ScopeComms allows an application to send user defined information to
     PVRTune via PVRPerfServer, both as counters and marks, or as editable data that can be
     passed back to the application.

PVRScope is supplied in the PVRScope.h header file. Your application also needs to link to the PVRScope 
library file, either a <tt>.lib</tt>, <tt>.so</tt>, or <tt>.dy</tt> file, depending on your platform.
 
For more information on PVRScope, see the <em>PVRScope User Manual</em>.

 @subsection limitStats PVRScopeStats Limitations
*****************************
@copydetails ScopeStats

 @subsection limitComms PVRScopeComms Limitations
*****************************
@copydetails ScopeComms
*/

#ifndef _PVRSCOPESTATS_H_
#define _PVRSCOPESTATS_H_

#ifdef WIN32
#define PVRSCOPE_EXPORT
#else
#define PVRSCOPE_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 @addtogroup ScopeStats PVRScopeStats
 @brief    The PVRScopeStats functionality of PVRScope is used to access the hardware performance counters in
           PowerVR hardware via a driver library called PVRScopeServices.
 @details  PVRScopeStats has the following limitations:
          \li Only one instance of @ref ScopeStats may communicate with PVRScopeServices at any
              given time. If a PVRScope enabled application attempts to communicate with
              PVRScopeServices at the same time as another such application, or at the same time as
              PVRPerfServer, conflicts can occur that may make performance data unreliable.
          \li Performance counters can only be read on devices whose drivers have been built with
              hardware profiling enabled. This configuration is the default in most production drivers due to negligible overhead.
          \li Performance counters contain the average value of that counter since the last time the counter was interrogated.
 @{
*/

/****************************************************************************
** Includes
****************************************************************************/

/****************************************************************************
** Enums
****************************************************************************/

/*!**************************************************************************
 @enum          EPVRScopeInitCode
 @brief         PVRScope initialisation return codes.
****************************************************************************/
enum EPVRScopeInitCode
{
    ePVRScopeInitCodeOk,							///< Initialisation OK
    ePVRScopeInitCodeOutOfMem,						///< Out of memory
    ePVRScopeInitCodeDriverSupportNotFound,			///< Driver support not found
    ePVRScopeInitCodeDriverSupportInsufficient,		///< Driver support insufficient
    ePVRScopeInitCodeDriverSupportInitFailed,		///< Driver support initialisation failed
    ePVRScopeInitCodeDriverSupportQueryInfoFailed	///< Driver support information query failed
};

/*!**************************************************************************
 @enum			EPVRScopeStandardCounter
 @brief			Set of "standard" counters, just a few of the total list of
				counters.
****************************************************************************/
enum EPVRScopeStandardCounter
{
	ePVRScopeStandardCounter_FPS,					///< Total device FPS
	ePVRScopeStandardCounter_Load_2D,				///< 2D core load
	ePVRScopeStandardCounter_Load_Renderer,			///< Renderer core load
	ePVRScopeStandardCounter_Load_Tiler,			///< Tiler core load
	ePVRScopeStandardCounter_Load_Compute,			///< Compute core load
	ePVRScopeStandardCounter_Load_Shader_Pixel,		///< Shader core load due to pixels
	ePVRScopeStandardCounter_Load_Shader_Vertex,	///< Shader core load due to vertices
	ePVRScopeStandardCounter_Load_Shader_Compute,	///< Shader core load due to compute
};

/*!**************************************************************************
 @enum			EPVRScopeEvent
 @brief			Set of PVRScope event types.
****************************************************************************/
enum EPVRScopeEvent
{
	ePVRScopeEventComputeBegin,					///< Compute begin
	ePVRScopeEventComputeEnd,					///< Compute end
	ePVRScopeEventTABegin,						///< TA begin
	ePVRScopeEventTAEnd,						///< TA end
	ePVRScopeEvent3DBegin,						///< 3D begin
	ePVRScopeEvent3DEnd,						///< 3D end
	ePVRScopeEvent2DBegin,						///< 2D begin
	ePVRScopeEvent2DEnd,						///< 2D end
	ePVRScopeEventRTUBegin,						///< RTU begin
	ePVRScopeEventRTUEnd,						///< RTU end
	ePVRScopeEventSHGBegin,						///< SHG begin
	ePVRScopeEventSHGEnd,						///< SHG end
};

/****************************************************************************
** Structures
****************************************************************************/

// Internal implementation data
struct SPVRScopeImplData;

/*!**************************************************************************
 @struct        SPVRScopeCounterDef
 @brief         Definition of a counter that PVRScope calculates.
****************************************************************************/
struct SPVRScopeCounterDef
{
    const char      *pszName;                   ///< Counter name, null terminated
    int				nBoolPercentage;			///< true if the counter is a percentage
    unsigned int    nGroup;                     ///< The counter group that the counter is in.
};

/*!**************************************************************************
 @struct        SPVRScopeCounterReading
 @brief         A set of return values resulting from querying the counter values.
****************************************************************************/
struct SPVRScopeCounterReading
{
    float           *pfValueBuf;                ///< Array of returned values
    unsigned int    nValueCnt;                  ///< Number of values set in the above array
    unsigned int    nReadingActiveGroup;        ///< Group that was active when counters were sampled
};

/*!**************************************************************************
 @struct        SPVRScopeGetInfo
 @brief         A set of return values holding miscellaneous PVRScope information.
****************************************************************************/
struct SPVRScopeGetInfo
{
    unsigned int    nGroupMax;                  ///< Highest group number of any counter
};

/*!**************************************************************************
@struct        SPVRScopeTimingPacket
@brief         A start or end time.
****************************************************************************/
struct SPVRScopeTimingPacket
{
	enum EPVRScopeEvent eEventType;  ///< Event type
	double              dTime;       ///< Event time (seconds)
	unsigned int        nPID;        ///< Event PID
};

/****************************************************************************
** Declarations
****************************************************************************/

PVRSCOPE_EXPORT
const char *PVRScopeGetDescription();           ///< Query the PVRScope library description


/*!**************************************************************************
 @brief         Initialise @ref ScopeStats, to access the HW performance counters in PowerVR.
 @return        EPVRScopeInitCodeOk on success.
****************************************************************************/
PVRSCOPE_EXPORT
enum EPVRScopeInitCode PVRScopeInitialise(
    struct SPVRScopeImplData **ppsData                 ///< Context data
);

/*!**************************************************************************	
 @brief         Shutdown or de-initalise @ref ScopeStats and free the allocated memory.
***************************************************************************/
PVRSCOPE_EXPORT
void PVRScopeDeInitialise(
    struct SPVRScopeImplData		**ppsData,          ///< Context data
	struct SPVRScopeCounterDef		**ppsCounters,      ///< Array of counters
	struct SPVRScopeCounterReading	* const psReading	///< Results memory area
);
    
/*!**************************************************************************
 @brief         Query for @ref ScopeStats information. This function should only be called during initialisation.
****************************************************************************/
PVRSCOPE_EXPORT
void PVRScopeGetInfo(
    struct SPVRScopeImplData	* const psData,         ///< Context data
	struct SPVRScopeGetInfo		* const psInfo          ///< Returned information
);
        
/*!**************************************************************************
 @brief         Query for the list of @ref ScopeStats HW performance counters, and
                allocate memory in which the counter values will be received. This function
                should only be called during initialisation.
****************************************************************************/
PVRSCOPE_EXPORT
int PVRScopeGetCounters(
	struct SPVRScopeImplData		* const psData,		///< Context data
	unsigned int					* const pnCount,	///< Returned number of counters
	struct SPVRScopeCounterDef		**ppsCounters,		///< Returned counter array
	struct SPVRScopeCounterReading	* const psReading	///< Pass a pointer to the structure to be initialised
);

/*!**************************************************************************
 @brief			Helper function to query for the counter index of one of a set
				of "standard" counters. The index will be into the results array
				(from PVRScopeReadCounters) not into the counter array (from
				PVRScopeGetCounters)
****************************************************************************/
PVRSCOPE_EXPORT
unsigned int PVRScopeFindStandardCounter(
	const unsigned int					nCount,				///< Returned number of counters, from PVRScopeGetCounters
	const struct SPVRScopeCounterDef	* const psCounters,	///< Returned counter array, from PVRScopeGetCounters
	const unsigned int					nGroup,				///< Group that will be active
	enum EPVRScopeStandardCounter		eCounter			///< Counter to be found
);

/*!**************************************************************************
 @brief	        Call regularly to allow PVRScope to track the latest hardware
                performance data. If psReading is not NULL, PVRScope will also
				calculate and return counter values to the application.
 @details       Returns 0 if no data is currently available; psReading will
				not be filled with valid data. Try again later.
				This function should be called "regularly"; two use cases are
                considered:
                1) A 3D application rendering a performance HUD (e.g. the on-
				screen graphs in PVRScopeExample). Such an application should
				call this function at least once per frame in order to gather
				new counter values. If slower HUD updates are desired,
				psReading may be NULL until a new reading is required, in
				order to smooth out values across longer time periods, perhaps
				a number of frames.
                2) A standalone performance monitor (e.g. PVRMonitor) or
				logging application. Such an application should idle and
				regularly wake up to call this function; suggested rates are
				100Hz (10ms delays) or 200Hz (5ms delays). If counter updates
				are required at a lower rate, set psReading to NULL on all
				calls except when new counter values are desired.
****************************************************************************/
PVRSCOPE_EXPORT
int PVRScopeReadCounters(
	struct SPVRScopeImplData		* const psData,		///< Context data
	struct SPVRScopeCounterReading	* const psReading	///< Returned data will be filled into the pointed-to structure
);

/*!**************************************************************************
 @brief	        Request a new HW counter group.
 @details       Changing the Active HW Group: the API is designed to allow the
				HW group to be changed immediately after gathering a reading.
****************************************************************************/
PVRSCOPE_EXPORT
void PVRScopeSetGroup(
	struct SPVRScopeImplData		* const psData,		///< Context data
	const unsigned int				nGroup	    		///< New group
);

/*!**************************************************************************
 @brief			Retrieve the timing data packets.
 @details       This function can be called periodically if you wish to access
				the start and end times of tasks running on the GPU.
				The first time this function is called will enable the feature;
				from then on data will be stored. If you wish to call this
				function once only at the end of a test run, call it once also
				prior to the test run.
****************************************************************************/
PVRSCOPE_EXPORT
const struct SPVRScopeTimingPacket *PVRScopeReadTimingData(
	struct SPVRScopeImplData	* const psData,	///< Context data
	unsigned int				* const pnCount	///< Returned number of packets
);

/*! @} */

#ifdef __cplusplus
}
#endif

#undef PVRSCOPE_EXPORT
#endif /* _PVRSCOPESTATS_H_ */

/*****************************************************************************
 End of file (PVRScopeStats.h)
*****************************************************************************/
