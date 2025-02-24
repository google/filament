///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dia2.cs                                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

namespace dia2
{
    using System;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [ComImport]
    [Guid("e6756135-1e65-4d17-8576-610761398c3c")]
    public class DiaDataSource
    {

    }

    [ComImport]
    [Guid("79F1BB5F-B66E-48e5-B6A9-1545C323CA3D")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaDataSource
    {
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_lastError();
        void loadDataFromPdb(string path);
        void loadAndValidateDataFromPdb(string path, ref Guid pcsig70, UInt32 sig, UInt32 age);
        void loadDataForExe(string executable, string searchPath, [MarshalAs(UnmanagedType.IUnknown)] object pCallback);
        void loadDataFromIStream(IStream pIStream);
        IDiaSession openSession();
        // loadDataFromCodeViewInfo
        // loadDataFromMiscInfo
    }

    [ComImport]
    [Guid("2F609EE1-D1C8-4E24-8288-3326BADCD211")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaSession
    {
        UInt64 get_loadAddress();
        void put_loadAddress(UInt64 value);
        IDiaSymbol get_globalScope();
        IDiaEnumTables getEnumTables();
        void getSymbolsByAddr();
        void findChildren();
        void findChildrenEx();
        void findChildrenExByAddr();
        void findChildrenExByVA();
        void findChildrenExByRVA();
        void findSymbolByAddr();
        void findSymbolByRVA();
        void findSymbolByVA();
        void findSymbolByToken();
        void symsAreEquiv();
        void symbolById();
        void findSymbolByRVAEx();
        void findSymbolByVAEx();
        void findFile();
        void findFileById();
        void findLines();
        void findLinesByAddr();
        void findLinesByRVA();
        void findLinesByVA();
        void findLinesByLinenum();
        object /*IDiaEnumInjectedSources*/ findInjectedSource(string srcFile);
        object /*IDiaEnumDebugStreams*/ getEnumDebugStreams();
        void findInlineFramesByAddr();
        void findInlineFramesByRVA();
        void findInlineFramesByVA();
        void findInlineeLines();
        void findInlineeLinesByAddr();
        void findInlineeLinesByRVA();
        void findInlineeLinesByVA();
        void findInlineeLinesByLinenum();
        void findInlineesByName();
        void findAcceleratorInlineeLinesByLinenum();
        void findSymbolsForAcceleratorPointerTag();
        void findSymbolsByRVAForAcceleratorPointerTag();
        void findAcceleratorInlineesByName();
        void addressForVA();
        void addressForRVA();
        void findILOffsetsByAddr();
        void findILOffsetsByRVA();
        void findILOffsetsByVA();
        void findInputAssemblyFiles();
        void findInputAssembly();
        void findInputAssemblyById();
        void getFuncMDTokenMapSize();
        void getFuncMDTokenMap();
        void getTypeMDTokenMapSize();
        void getTypeMDTokenMap();
        void getNumberOfFunctionFragments_VA();
        void getNumberOfFunctionFragments_RVA();
        void getFunctionFragments_VA();
        void getFunctionFragments_RVA();
        object /*IDiaEnumSymbols*/ getExports();
        object /*IDiaEnumSymbols*/ getHeapAllocationSites();
        void findInputAssemblyFile();
    }

    [ComImport]
    [Guid("cb787b2f-bd6c-4635-ba52-933126bd2dcd")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaSymbol
    {
        UInt32 get_symIndexId();
        UInt32 get_symTag();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_name();
        IDiaSymbol get_lexicalParent();
        IDiaSymbol get_classParent();
        IDiaSymbol get_type();
        UInt32 get_dataKind();
        UInt32 get_locationType();
        UInt32 get_addressSection();
        UInt32 get_addressOffset();
        UInt32 get_relativeVirtualAddress();
        UInt64 get_virtualAddress();
        UInt32 get_registerId();
        Int32 get_offset();
        UInt64 get_length();
        UInt32 get_slot();
        bool get_volatileType();
        bool get_constType();
        bool get_unalignedType();
        UInt32 get_access();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_libraryName();
        UInt32 get_platform();
        UInt32 get_language();
        bool get_editAndContinueEnabled();
        UInt32 get_frontEndMajor();
        UInt32 get_frontEndMinor();
        UInt32 get_frontEndBuild();
        UInt32 get_backEndMajor();
        UInt32 get_backEndMinor();
        UInt32 get_backEndBuild();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_sourceFileName();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_unused();
        UInt32 get_thunkOrdinal();
        Int32 get_thisAdjust();
        UInt32 get_virtualBaseOffset();
        bool get_virtual();
        bool get_intro();
        bool get_pure();
        UInt32 get_callingConvention();
        [return: MarshalAs(UnmanagedType.AsAny)] // VARIANT
        object get_value();
        UInt32 get_baseType();
        UInt32 get_token();
        UInt32 get_timeStamp();
        Guid get_guid();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_symbolsFileName();
        bool get_reference();
        UInt32 get_count();
        UInt32 get_bitPosition();
        IDiaSymbol get_arrayIndexType();
        bool get_packed();
        bool get_constructor();
        bool get_overloadedOperator();
        bool get_nested();
        bool get_hasNestedTypes();
        bool get_hasAssignmentOperator();
        bool get_hasCastOperator();
        bool get_scoped();
        bool get_virtualBaseClass();
        bool get_indirectVirtualBaseClass();
        Int32 get_virtualBasePointerOffset();
        IDiaSymbol get_virtualTableShape();
        UInt32 get_lexicalParentId();
        UInt32 get_classParentId();
        UInt32 get_typeId();
        UInt32 get_arrayIndexTypeId();
        UInt32 get_virtualTableShapeId();
        bool get_code();
        bool get_function();
        bool get_managed();
        bool get_msil();
        UInt32 get_virtualBaseDispIndex();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_undecoratedName();
        UInt32 get_age();
        UInt32 get_signature();
        bool get_compilerGenerated();
        bool get_addressTaken();
        UInt32 get_rank();
        IDiaSymbol get_lowerBound();
        IDiaSymbol get_upperBound();
        UInt32 get_lowerBoundId();
        UInt32 get_upperBoundId();
        void get_dataBytes(UInt32 cbData, out UInt32 pcbData, out byte[] pbData);
        void findChildren();
        void findChildrenEx();
        void findChildrenExByAddr();
        void findChildrenExByVA();
        void findChildrenExByRVA();
        UInt32 get_targetSection();
        UInt32 get_targetOffset();
        UInt32 get_targetRelativeVirtualAddress();
        UInt64 get_targetVirtualAddress();
        UInt32 get_machineType();
        UInt32 get_oemId();
        UInt32 get_oemSymbolId();
        void get_types();
        void get_typeIds();
        IDiaSymbol get_objectPointerType();
        UInt32 get_udtKind();
        void get_undecoratedNameEx();
        bool get_noReturn();
        bool get_customCallingConvention();
        bool get_noInline();
        bool get_optimizedCodeDebugInfo();
        bool get_notReached();
        bool get_interruptReturn();
        bool get_farReturn();
        bool get_isStatic();
        bool get_hasDebugInfo();
        bool get_isLTCG();
        bool get_isDataAligned();
        bool get_hasSecurityChecks();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_compilerName();
        bool get_hasAlloca();
        bool get_hasSetJump();
        bool get_hasLongJump();
        bool get_hasInlAsm();
        bool get_hasEH();
        bool get_hasSEH();
        bool get_hasEHa();
        bool get_isNaked();
        bool get_isAggregated();
        bool get_isSplitted();
        IDiaSymbol get_container();
        bool get_inlSpec();
        bool get_noStackOrdering();
        IDiaSymbol get_virtualBaseTableType();
        bool get_hasManagedCode();
        bool get_isHotpatchable();
        bool get_isCVTCIL();
        bool get_isMSILNetmodule();
        bool get_isCTypes();
        bool get_isStripped();
        UInt32 get_frontEndQFE();
        UInt32 get_backEndQFE();
        bool get_wasInlined();
        bool get_strictGSCheck();
        bool get_isCxxReturnUdt();
        bool get_isConstructorVirtualBase();
        bool get_RValueReference();
        IDiaSymbol get_unmodifiedType();
        bool get_framePointerPresent();
        bool get_isSafeBuffers();
        bool get_intrinsic();
        bool get_sealed();
        bool get_hfaFloat();
        bool get_hfaDouble();
        UInt32 get_liveRangeStartAddressSection();
        UInt32 get_liveRangeStartAddressOffset();
        UInt32 get_liveRangeStartRelativeVirtualAddress();
        UInt32 get_countLiveRanges();
        UInt64 get_liveRangeLength();
        UInt32 get_offsetInUdt();
        UInt32 get_paramBasePointerRegisterId();
        UInt32 get_localBasePointerRegisterId();
        bool get_isLocationControlFlowDependent();
        UInt32 get_stride();
        UInt32 get_numberOfRows();
        UInt32 get_numberOfColumns();
        bool get_isMatrixRowMajor();
        void get_numericProperties();
        void get_modifierValues();
        bool get_isReturnValue();
        bool get_isOptimizedAway();
        UInt32 get_builtInKind();
        UInt32 get_registerType();
        UInt32 get_baseDataSlot();
        UInt32 get_baseDataOffset();
        UInt32 get_textureSlot();
        UInt32 get_samplerSlot();
        UInt32 get_uavSlot();
        UInt32 get_sizeInUdt();
        UInt32 get_memorySpaceKind();
        UInt32 get_unmodifiedTypeId();
        UInt32 get_subTypeId();
        IDiaSymbol get_subType();
        UInt32 get_numberOfModifiers();
        UInt32 get_numberOfRegisterIndices();
        bool get_isHLSLData();
        bool get_isPointerToDataMember();
        bool get_isPointerToMemberFunction();
        bool get_isSingleInheritance();
        bool get_isMultipleInheritance();
        bool get_isVirtualInheritance();
        bool get_restrictedType();
        bool get_isPointerBasedOnSymbolValue();
        IDiaSymbol get_baseSymbol();
        UInt32 get_baseSymbolId();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_objectFileName();
        bool get_isAcceleratorGroupSharedLocal();
        bool get_isAcceleratorPointerTagLiveRange();
        bool get_isAcceleratorStubFunction();
        UInt32 get_numberOfAcceleratorPointerTags();
        bool get_isSdl();
        bool get_isWinRTPointer();
        bool get_isRefUdt();
        bool get_isValueUdt();
        bool get_isInterfaceUdt();
        void findInlineFramesByAddr();
        void findInlineFramesByRVA();
        void findInlineFramesByVA();
        void findInlineeLines();
        void findInlineeLinesByAddr();
        void findInlineeLinesByRVA();
        void findInlineeLinesByVA();
        void findSymbolsForAcceleratorPointerTag();
        void findSymbolsByRVAForAcceleratorPointerTag();
        void get_acceleratorPointerTags();
        void getSrcLineOnTypeDefn();
        bool get_isPGO();
        bool get_hasValidPGOCounts();
        bool get_isOptimizedForSpeed();
        UInt32 get_PGOEntryCount();
        UInt32 get_PGOEdgeCount();
        UInt64 get_PGODynamicInstructionCount();
        UInt32 get_staticSize();
        UInt32 get_finalLiveStaticSize();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_phaseName();
        bool get_hasControlFlowCheck();
        bool get_constantExport();
        bool get_dataExport();
        bool get_privateExport();
        bool get_noNameExport();
        bool get_exportHasExplicitlyAssignedOrdinal();
        bool get_exportIsForwarder();
        UInt32 get_ordinal();
        UInt32 get_frameSize();
        UInt32 get_exceptionHandlerAddressSection();
        UInt32 get_exceptionHandlerAddressOffset();
        UInt32 get_exceptionHandlerRelativeVirtualAddress();
        UInt64 get_exceptionHandlerVirtualAddress();
        void findInputAssemblyFile();
        UInt32 get_characteristics();
        IDiaSymbol get_coffGroup();
        UInt32 get_bindID();
        UInt32 get_bindSpace();
        UInt32 get_bindSlot();
    }

    [ComImport]
    [Guid("C65C2B0A-1150-4d7a-AFCC-E05BF3DEE81E")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaEnumTables
    {
        void get__NewEnum();
        UInt32 get_Count();
        IDiaTable Item(object index);
        UInt32 Next(UInt32 count,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Interface, SizeParamIndex = 2)]
            ref IDiaTable[] tables, out UInt32 fetched);
        void Skip(UInt32 count);
        void Reset();
        IDiaEnumTables Clone();
    }

    [ComImport]
    [Guid("B388EB14-BE4D-421d-A8A1-6CF7AB057086")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaLineNumber
    {
        IDiaSymbol get_compiland();
        IDiaSourceFile get_sourceFile();
        uint get_lineNumber();
        uint get_lineNumberEnd();
        uint get_columnNumber();
        uint get_columnNumberEnd();
        uint get_addressSection();
        uint get_addressOffset();
        uint get_relativeVirtualAddress();
        UInt64 get_virtualAddress();
        uint get_length();
        uint get_sourceFileId();
        bool get_statement();
        uint get_compilandId();
    }

    [ComImport]
    [Guid("0CF4B60E-35B1-4c6c-BDD8-854B9C8E3857")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaSectionContrib
    {
        IDiaSymbol get_compiland();
        uint get_addressSection();
        uint get_addressOffset();
        uint get_relativeVirtualAddress();
        UInt64 get_virtualAddress();
        uint get_length();
        bool get_notPaged();
        bool get_code();
        bool get_initializedData();
        bool get_uninitializedData();
        bool get_remove();
        bool get_comdat();
        bool get_discardable();
        bool get_notCached();
        bool get_share();
        bool get_execute();
        bool get_read();
        bool get_write();
        uint get_dataCrc();
        uint get_relocationsCrc();
        uint get_compilandId();
        bool get_code16bit();
    }

    [ComImport]
    [Guid("0775B784-C75B-4449-848B-B7BD3159545B")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaSegment
    {
        uint get_frame();
        uint get_offset();
        uint get_length();
        bool get_read();
        bool get_write();
        bool get_execute();
        uint get_addressSection();
        uint get_relativeVirtualAddress();
        UInt64 get_virtualAddress();
    }

    [ComImport]
    [Guid("00000100-0000-0000-C000-000000000046")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IEnumUnknown
    {
        UInt32 Next(UInt32 count,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.IUnknown, SizeParamIndex = 2)]
            ref object[] tables, out UInt32 fetched);
        void Skip(UInt32 count);
        void Reset();
        IEnumUnknown Clone();
    }

    [ComImport]
    [Guid("A2EF5353-F5A8-4eb3-90D2-CB526ACB3CDD")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaSourceFile
    {
        uint get_uniqueId();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_fileName();
        void /* IDiaEnumSymbols */ get_compilands();
        [PreserveSig] int get_checksum(uint cbData, out uint pcbData,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.I1, SizeParamIndex = 1)] ref byte[] pbData);
    }

    [ComImport]
    [Guid("AE605CDC-8105-4a23-B710-3259F1E26112")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaInjectedSource
    {
        uint get_crc();
        uint get_length();

        [return: MarshalAs(UnmanagedType.BStr)]
        string get_fileName();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_objectFilename();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_virtualFilename();
        uint get_sourceCompression();
        [PreserveSig]
        int get_source(uint cbData, out uint pcbData,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U8, SizeParamIndex = 0)] byte[] pbData);

    }

    [ComImport]
    [Guid("4A59FB77-ABAC-469b-A30B-9ECC85BFEF14")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDiaTable // : IEnumUnknown - need to replay vtable
    {
        UInt32 Next(UInt32 count,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.IUnknown, SizeParamIndex = 2)]
            ref object[] tables, out UInt32 fetched);
        void Skip(UInt32 count);
        void Reset();
        IEnumUnknown Clone();

        [return: MarshalAs(UnmanagedType.IUnknown)]
        object get__NewEnum();
        [return: MarshalAs(UnmanagedType.BStr)]
        string get_name();
        Int32 get_Count();
        [return: MarshalAs(UnmanagedType.IUnknown)]
        object Item(UInt32 index);
    }

    public enum SymTagEnum
    {
        SymTagNull,
        SymTagExe,
        SymTagCompiland,
        SymTagCompilandDetails,
        SymTagCompilandEnv,
        SymTagFunction,
        SymTagBlock,
        SymTagData,
        SymTagAnnotation,
        SymTagLabel,
        SymTagPublicSymbol,
        SymTagUDT,
        SymTagEnum,
        SymTagFunctionType,
        SymTagPointerType,
        SymTagArrayType,
        SymTagBaseType,
        SymTagTypedef,
        SymTagBaseClass,
        SymTagFriend,
        SymTagFunctionArgType,
        SymTagFuncDebugStart,
        SymTagFuncDebugEnd,
        SymTagUsingNamespace,
        SymTagVTableShape,
        SymTagVTable,
        SymTagCustom,
        SymTagThunk,
        SymTagCustomType,
        SymTagManagedType,
        SymTagDimension,
        SymTagCallSite,
        SymTagInlineSite,
        SymTagBaseInterface,
        SymTagVectorType,
        SymTagMatrixType,
        SymTagHLSLType,
        SymTagCaller,
        SymTagCallee,
        SymTagExport,
        SymTagHeapAllocationSite,
        SymTagCoffGroup,
        SymTagMax
    };
}
