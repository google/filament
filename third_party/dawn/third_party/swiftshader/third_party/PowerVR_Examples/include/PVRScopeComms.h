/*!***********************************************************************

 @file           PVRScopeComms.h
 @copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved.
 @brief          PVRScopeComms header file. @copybrief ScopeComms
 
**************************************************************************/

#ifndef _PVRSCOPECOMMS_H_
#define _PVRSCOPECOMMS_H_

#ifdef _WIN32
#define PVRSCOPE_EXPORT
#else
#define PVRSCOPE_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
	PVRPerfServer and PVRTune communications
*/

/*!
 @addtogroup ScopeComms PVRScopeComms
 @brief      The PVRScopeComms functionality of PVRScope allows an application to send user defined information to
             PVRTune via PVRPerfServer, both as counters and marks, or as editable data that can be
             passed back to the application.
 @details    PVRScopeComms has the following limitations:
             \li PVRPerfServer must be running on the host device if a @ref ScopeComms enabled
                 application wishes to send custom counters or marks to PVRTune. If the application in
                 question also wishes to communicate with PVRScopeServices without experiencing any
                 undesired behaviour PVRPerfServer should be run with the '--disable-hwperf' flag.
             \li The following types may be sent: Boolean, Enumerator, Float, Integer, String.
 @{
*/

/****************************************************************************
** Enums
****************************************************************************/
    
/*!**************************************************************************
 @enum          ESPSCommsLibType
 @brief         Each editable library item has a data type associated with it
****************************************************************************/
/// 
enum ESPSCommsLibType
{
	eSPSCommsLibTypeString,		///< data is string (NOT NULL-terminated, use length parameter)
	eSPSCommsLibTypeFloat,		///< data is SSPSCommsLibraryTypeFloat
	eSPSCommsLibTypeInt,		///< data is SSPSCommsLibraryTypeInt
	eSPSCommsLibTypeEnum,		///< data is string (NOT NULL-terminated, use length parameter). First line is selection number, subsequent lines are available options.
	eSPSCommsLibTypeBool		///< data is SSPSCommsLibraryTypeBool
};

/****************************************************************************
** Structures
****************************************************************************/

// Internal implementation data
struct SSPSCommsData;
    
/*!**************************************************************************
 @struct        SSPSCommsLibraryItem
 @brief         Definition of one editable library item
****************************************************************************/
struct SSPSCommsLibraryItem
{
	
	const char				*pszName;       ///< Item name. If dots are used, PVRTune could show these as a foldable tree view.
	unsigned int			nNameLength;	///< Item name length

	enum ESPSCommsLibType	eType;			///< Item type

	const char				*pData;         ///< Item data
	unsigned int			nDataLength;	///< Item data length
};
    
/*!**************************************************************************
 @struct        SSPSCommsLibraryTypeFloat
 @brief         Current, minimum and maximum values for an editable library item of type float
****************************************************************************/
struct SSPSCommsLibraryTypeFloat
{
	float fCurrent;						///< Current value
	float fMin;							///< Minimal value
	float fMax;							///< Maximum value
};
    
/*!**************************************************************************
 @struct        SSPSCommsLibraryTypeInt
 @brief         Current, minimum and maximum values for an editable library item of type int
****************************************************************************/
struct SSPSCommsLibraryTypeInt
{
	int nCurrent;						///< Current value
	int nMin;    						///< Minimal value
	int nMax;    						///< Maximum value
};
    
/*!**************************************************************************
 @struct        SSPSCommsLibraryTypeBool 
 @brief         Current value for an editable library item of type bool
****************************************************************************/
struct SSPSCommsLibraryTypeBool
{
	int nBoolValue;						///< Boolean value (zero = false)
};
    
/*!**************************************************************************
 @struct        SSPSCommsCounterDef
 @brief         Definition of one custom counter
****************************************************************************/
struct SSPSCommsCounterDef
{
	const char			*pszName;		    ///< Custom counter name
	unsigned int		nNameLength;	    ///< Custom counter name length
};

/****************************************************************************
** Declarations
****************************************************************************/

    
/*!**************************************************************************
 @brief         Initialise @ref ScopeComms
 @return        @ref ScopeComms data.
****************************************************************************/
PVRSCOPE_EXPORT
struct SSPSCommsData *pplInitialise(
	const char			* const psName,		///< String to describe the application
	const unsigned int	nNameLen			///< String length
);
    
/*!**************************************************************************
 @brief         Shutdown or de-initialise the remote control section of PVRScope.
****************************************************************************/
PVRSCOPE_EXPORT
void pplShutdown(
    struct SSPSCommsData	*psData			///< Context data
);

/*!**************************************************************************
 @brief         Optional function. Sleeps until there is a connection to
				PVRPerfServer, or time-out. Normally, each thread will wait for
				its own connection, and each time-out will naturally happen in
				parallel. But if a thread happens to have multiple connections,
				N, then waiting for them all [in serial] with time-out M would
				take N*M ms if they were all to time-out (e.g. PVRPerfServer is
				not running); therefore this function, is designed to allow an
				entire array of connections to be waited upon simultaneously.
****************************************************************************/
PVRSCOPE_EXPORT
void pplWaitForConnection(
	struct SSPSCommsData	* const psData,			///< Array of context data pointers
	int						* const pnBoolResults,	///< Array of results - false (0) if timeout
	const unsigned int		nCount,					///< Array length
	const unsigned int		nTimeOutMS				///< Time-out length in milliseconds
);

/*!**************************************************************************
 @brief         Query for the time. Units are microseconds, resolution is undefined.
****************************************************************************/
PVRSCOPE_EXPORT
unsigned int pplGetTimeUS(
	struct SSPSCommsData	* const psData	    ///< Context data
);

/*!**************************************************************************
 @brief         Send a time-stamped string marker to be displayed in PVRTune.
 @details       Examples might be:
                \li switching to outdoor renderer
                \li starting benchmark test N
****************************************************************************/
PVRSCOPE_EXPORT
int pplSendMark(
	struct SSPSCommsData	* const psData,		///< Context data
	const char				* const psString,	///< String to send
	const unsigned int		nLen				///< String length
);

/*!**************************************************************************
 @brief         Send a time-stamped begin marker to PVRTune. 
 @details       Every begin must at some point be followed by an end; begin/end
				pairs can be nested. PVRTune will show these as an activity
				timeline, using a "flame graph" style when there is nesting.
				See also the CPPLProcessingScoped helper class.
****************************************************************************/
PVRSCOPE_EXPORT
int pplSendProcessingBegin(
	struct SSPSCommsData	* const psData,		///< Context data
	const char				* const	psString,	///< Name of the processing block
	const unsigned int		nLen,				///< String length
	const unsigned int		nFrame				///< Iteration (or frame) number, by which processes can be grouped.
);

/*!**************************************************************************
 @brief         Send a time-stamped end marker to PVRTune. 
 @details       Every begin must at some point be followed by an end; begin/end
				pairs can be nested. PVRTune will show these as an activity
				timeline, using a "flame graph" style when there is nesting.
				See also the CPPLProcessingScoped helper class.
****************************************************************************/
PVRSCOPE_EXPORT
int pplSendProcessingEnd(
	struct SSPSCommsData	* const psData      ///< Context data
);

/*!**************************************************************************
 @brief         Create a library of remotely editable items
****************************************************************************/
PVRSCOPE_EXPORT
int pplLibraryCreate(
	struct SSPSCommsData				* const psData,	///< Context data
	const struct SSPSCommsLibraryItem	* const pItems,	///< Editable items
	const unsigned int					nItemCount		///< Number of items
);

/*!**************************************************************************
 @brief	        Query to see whether a library item has been edited, and also
				retrieve the new data.
****************************************************************************/
PVRSCOPE_EXPORT
int pplLibraryDirtyGetFirst(
	struct SSPSCommsData	* const psData,	        ///< Context data
	unsigned int			* const pnItem,	        ///< Item number
	unsigned int			* const pnNewDataLen,   ///< New data length
	const char				**ppData        	    ///< New data
);

/*!**************************************************************************
 @brief         Specify the number of custom counters and their definitions
****************************************************************************/
PVRSCOPE_EXPORT
int pplCountersCreate(
	struct SSPSCommsData				* const psData,	        ///< Context data
	const struct SSPSCommsCounterDef	* const psCounterDefs,  ///< Counter definitions
	const unsigned int					nCount    	            ///< Number of counters
);

/*!**************************************************************************
 @brief 	    Send an update for all the custom counters. The
				psCounterReadings array must be nCount long.
****************************************************************************/
PVRSCOPE_EXPORT
int pplCountersUpdate(
	struct SSPSCommsData		* const psData,	            ///< Context data
	const unsigned int			* const psCounterReadings	///< Counter readings array
);
        
/*!**************************************************************************
 @brief	        Force a cache flush.
 @details       Some implementations store data sends in the cache. If the data
				rate is low, the real send of data can be significantly
				delayed.
                If it is necessary to flush the cache, the best results are
				likely to be achieved by calling this function with a frequency
				between once per second up to once per frame. If data is sent
				extremely infrequently, this function could be called once at
				the end of each bout of data send.
****************************************************************************/
PVRSCOPE_EXPORT
int pplSendFlush(
	struct SSPSCommsData		* const psData	    ///< Context data
);

/*! @} */

#ifdef __cplusplus
}

/*!**************************************************************************
@class			CPPLProcessingScoped
@brief			Helper class which will send a processing begin/end pair around
				its scope. You would typically instantiate these at the top of
				a function or after the opening curly-brace of a new scope
				within a function.
****************************************************************************/
class CPPLProcessingScoped
{
protected:
	SSPSCommsData		* const m_psData;	    ///< Context data

public:
	CPPLProcessingScoped(
		SSPSCommsData		* const psData,		///< Context data
		const char			* const	psString,	///< Name of the processing block
		const unsigned int	nLen,				///< String length
		const unsigned int	nFrame=0			///< Iteration (or frame) number, by which processes can be grouped.
		)
		: m_psData(psData)
	{
		if (m_psData)
			pplSendProcessingBegin(m_psData, psString, nLen, nFrame);
	}

	~CPPLProcessingScoped()
	{
		if (m_psData)
			pplSendProcessingEnd(m_psData);
	}

private:
	CPPLProcessingScoped(const CPPLProcessingScoped&);				// Prevent copy-construction
	CPPLProcessingScoped& operator=(const CPPLProcessingScoped&);	// Prevent assignment
};
#endif

#undef PVRSCOPE_EXPORT
#endif /* _PVRSCOPECOMMS_H_ */

/*****************************************************************************
 End of file (PVRScopeComms.h)
*****************************************************************************/
