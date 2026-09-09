///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DotNetDxc.cs                                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides P/Invoke declarations for dxcompiler HLSL support.               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#region Namespaces.

using System;
using System.Text;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#endregion Namespaces.

namespace DotNetDxc
{
    public enum DxcGlobalOptions : uint
    {
        DxcGlobalOpt_None = 0x0,
        DxcGlobalOpt_ThreadBackgroundPriorityForIndexing = 0x1,
        DxcGlobalOpt_ThreadBackgroundPriorityForEditing = 0x2,
        DxcGlobalOpt_ThreadBackgroundPriorityForAll
    }

    [Flags]
    public enum DxcValidatorFlags : uint
    {
        Default = 0,
        InPlaceEdit = 1,
        ValidMask = 1
    }

    [Flags]
    public enum DxcVersionInfoFlags : uint
    {
        None = 0,
        Debug = 1
    }

    /// <summary>
    /// A cursor representing some element in the abstract syntax tree for
    /// a translation unit.
    /// </summary>
    /// <remarks>
    /// The cursor abstraction unifies the different kinds of entities in a
    /// program--declaration, statements, expressions, references to declarations,
    /// etc.--under a single "cursor" abstraction with a common set of operations.
    /// Common operation for a cursor include: getting the physical location in
    /// a source file where the cursor points, getting the name associated with a
    /// cursor, and retrieving cursors for any child nodes of a particular cursor.
    /// <remarks>
    [ComImport]
    [Guid("1467b985-288d-4d2a-80c1-ef89c42c40bc")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcCursor
    {
        IDxcSourceRange GetExtent();
        IDxcSourceLocation GetLocation();
        // <summary>Describes what kind of construct this cursor refers to.</summary>
        DxcCursorKind GetCursorKind();
        DxcCursorKindFlags GetCursorKindFlags();
        IDxcCursor GetSemanticParent();
        IDxcCursor GetLexicalParent();
        IDxcType GetCursorType();
        int GetNumArguments();
        IDxcCursor GetArgumentAt(int index);
        IDxcCursor GetReferencedCursor();
        IDxcCursor GetDefinitionCursor();
        void FindReferencesInFile(IDxcFile file, UInt32 skip, UInt32 top, out uint resultLength,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Interface, SizeParamIndex = 3)]
            out IDxcCursor[] cursors);
        [return: MarshalAs(UnmanagedType.LPStr)]
        string GetSpelling();
        bool IsEqualTo(IDxcCursor other);
        bool IsNull();
    }

    [ComImport]
    [Guid("4f76b234-3659-4d33-99b0-3b0db994b564")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcDiagnostic
    {
        [return: MarshalAs(UnmanagedType.LPStr)]
        string FormatDiagnostic(DxcDiagnosticDisplayOptions options);
        DxcDiagnosticSeverity GetSeverity();
        IDxcSourceLocation GetLocation();
        [return: MarshalAs(UnmanagedType.LPStr)]
        string GetSpelling();
        [return: MarshalAs(UnmanagedType.LPStr)]
        string GetCategoryText();
        UInt32 GetNumRanges();
        IDxcSourceRange GetRangeAt(UInt32 index);
        UInt32 GetNumFixIts();
        [return: MarshalAs(UnmanagedType.LPStr)]
        string GetFixItAt(UInt32 index, out IDxcSourceRange range);
    }

    /// <summary>
    /// Use this interface to represent a file (saved or in-memory).
    /// </summary>
    [ComImport]
    [Guid("bb2fca9e-1478-47ba-b08c-2c502ada4895")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcFile
    {
        [return: MarshalAs(UnmanagedType.LPStr)]
        string GetName();
    }

    [ComImport]
    [Guid("b1f99513-46d6-4112-8169-dd0d6053f17d")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcIntelliSense
    {
        IDxcIndex CreateIndex();
        IDxcSourceLocation GetNullLocation();
        IDxcSourceRange GetNullRange();
        IDxcSourceRange GetRange(IDxcSourceLocation start, IDxcSourceLocation end);
        DxcDiagnosticDisplayOptions GetDefaultDiagnosticDisplayOptions();
        DxcTranslationUnitFlags GetDefaultEditingTUOptions();
    }

    /// <summary>
    /// Use this interface to represent the context in which translation units are parsed.
    /// </summary>
    [ComImport]
    [Guid("937824a0-7f5a-4815-9ba7-7fc0424f4173")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcIndex
    {
        void SetGlobalOptions(DxcGlobalOptions options);
        DxcGlobalOptions GetGlobalOptions();
        IDxcTranslationUnit ParseTranslationUnit(
          [MarshalAs(UnmanagedType.LPStr)] string source_filename,
          [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] commandLineArgs,
          int num_command_line_args,
          [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Interface)] IDxcUnsavedFile[] unsavedFiles,
          uint num_unsaved_files,
          uint options);
    }

    /// <summary>
    /// Describes a location in a source file.
    /// </summary>
    [Guid("8e7ddf1c-d7d3-4d69-b286-85fccba1e0cf")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcSourceLocation
    {
        bool IsEqualTo(IDxcSourceLocation other);
        void GetSpellingLocation(out IDxcFile file, out UInt32 line, out UInt32 col, out UInt32 offset);
    }

    /// <summary>
    /// Describes a range of text in a source file.
    /// </summary>
    [ComImport]
    [Guid("f1359b36-a53f-4e81-b514-b6b84122a13f")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcSourceRange
    {
        bool IsNull();
        IDxcSourceLocation GetStart();
        IDxcSourceLocation GetEnd();
    }

    /// <summary>
    /// Describes a single preprocessing token.
    /// </summary>
    [ComImport]
    [Guid("7f90b9ff-a275-4932-97d8-3cfd234482a2")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcToken
    {
        DxcTokenKind GetKind();
        IDxcSourceLocation GetLocation();
        IDxcSourceRange GetExtent();
        [return: MarshalAs(UnmanagedType.LPStr)]
        string GetSpelling();
    }

    [ComImport]
    [Guid("9677dee0-c0e5-46a1-8b40-3db3168be63d")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcTranslationUnit
    {
        IDxcCursor GetCursor();
        void Tokenize(IDxcSourceRange range,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Interface, SizeParamIndex=2)]
            out IDxcToken[] tokens,
            out uint tokenCount);
        IDxcSourceLocation GetLocation(IDxcFile file, UInt32 line, UInt32 column);
        UInt32 GetNumDiagnostics();
        IDxcDiagnostic GetDiagnosticAt(UInt32 index);
        IDxcFile GetFile([MarshalAs(UnmanagedType.LPStr)]string name);
        [return: MarshalAs(UnmanagedType.LPStr)]
        string GetFileName();
        void Reparse(
          [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Interface)] IDxcUnsavedFile[] unsavedFiles,
          uint num_unsaved_files);
        IDxcCursor GetCursorForLocation(IDxcSourceLocation location);
        IDxcSourceLocation GetLocationForOffset(IDxcFile file, UInt32 offset);
        void GetSkippedRanges(IDxcFile file,
            out uint rangeCount,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Interface, SizeParamIndex=1)]
            out IDxcSourceRange[] ranges);
        void GetDiagnosticDetails(UInt32 index, DxcDiagnosticDisplayOptions options,
            out UInt32 errorCode,
            out UInt32 errorLine,
            out UInt32 errorColumn,
            [MarshalAs(UnmanagedType.BStr)]
            out string errorFile,
            out UInt32 errorOffset,
            out UInt32 errorLength,
            [MarshalAs(UnmanagedType.BStr)]
            out string errorMessage);
    }

    [ComImport]
    [Guid("2ec912fd-b144-4a15-ad0d-1c5439c81e46")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcType
    {
        [return: MarshalAs(UnmanagedType.LPStr)]
        string GetSpelling();
        bool IsEqualTo(IDxcType other);
    };


    [ComImport]
    [Guid("8ec00f98-07d0-4e60-9d7c-5a50b5b0017f")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcUnsavedFile
    {
        void GetFileName([MarshalAs(UnmanagedType.LPStr)] out string value);
        void GetContents([MarshalAs(UnmanagedType.LPStr)] out string value);
        void GetLength(out UInt32 length);
    }

    [ComImport]
    [Guid("A6E82BD2-1FD7-4826-9811-2857E797F49A")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcValidator
    {
        IDxcOperationResult Validate(IDxcBlob shader, DxcValidatorFlags flags);
    }

    [ComImport]
    [Guid("b04f5b50-2059-4f12-a8ff-a1e0cde1cc7e")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcVersionInfo
    {
        void GetVersion(out UInt32 major, out UInt32 minor);
        DxcVersionInfoFlags GetFlags();
    }

    [ComImport]
    [Guid("c012115b-8893-4eb9-9c5a-111456ea1c45")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcRewriter
    {
        IDxcRewriteResult RemoveUnusedGlobals(
            IDxcBlobEncoding pSource,
            [MarshalAs(UnmanagedType.LPWStr)]string pEntryPoint,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Struct)] DXCDefine[] pDefines,
            uint defineCount);
        IDxcRewriteResult RewriteUnchanged(
            IDxcBlobEncoding pSource,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Struct)] DXCDefine[] pDefines,
            uint defineCount);

        IDxcRewriteResult RewriteUnchangedWithInclude(IDxcBlobEncoding pSource,
             [MarshalAs(UnmanagedType.LPWStr)] string pName,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.Struct)] DXCDefine[] pDefines,
            uint defineCount, IDxcIncludeHandler includeHandler, uint rewriteOption);
    }

    [ComImport]
    [Guid("CEDB484A-D4E9-445A-B991-CA21CA157DC2")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcRewriteResult
    {
        uint GetStatus();
        IDxcBlobEncoding GetRewrite();
        IDxcBlobEncoding GetErrorBuffer();
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct DXCEncodedText
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string pText;
        public uint Size;
        public uint CodePage; //should always be UTF-8 for this use
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct DXCDefine
    {
        [MarshalAs(UnmanagedType.LPWStr)]
        public string pName;
        [MarshalAs(UnmanagedType.LPWStr)]
        public string pValue;
    }

    public class TrivialDxcUnsavedFile : IDxcUnsavedFile
    {
        private readonly string fileName;
        private readonly string contents;

        public TrivialDxcUnsavedFile(string fileName, string contents)
        {
            //System.Diagnostics.Debug.Assert(fileName != null);
            //System.Diagnostics.Debug.Assert(contents != null);
            this.fileName = fileName;
            this.contents = contents;
        }

        public void GetFileName(out string value) { value = this.fileName; }
        public void GetContents(out string value) { value = this.contents; }
        public void GetLength(out UInt32 length) { length = (UInt32)this.contents.Length; }
    }

    public delegate int DxcCreateInstanceFn(ref Guid clsid, ref Guid iid, [MarshalAs(UnmanagedType.IUnknown)] out object instance);

    public class DefaultDxcLib
    {
        [DllImport(@"dxcompiler.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern int DxcCreateInstance(
            ref Guid clsid,
            ref Guid iid,
            [MarshalAs(UnmanagedType.IUnknown)] out object instance);

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        internal static DxcCreateInstanceFn GetDxcCreateInstanceFn()
        {
            return DxcCreateInstance;
        }
    }

    public class HlslDxcLib
    {
        private static Guid CLSID_DxcAssembler = new Guid("D728DB68-F903-4F80-94CD-DCCF76EC7151");
        private static Guid CLSID_DxcDiaDataSource = new Guid("CD1F6B73-2AB0-484D-8EDC-EBE7A43CA09F");
        private static Guid CLSID_DxcIntelliSense = new Guid("3047833c-d1c0-4b8e-9d40-102878605985");
        private static Guid CLSID_DxcRewriter = new Guid("b489b951-e07f-40b3-968d-93e124734da4");
        private static Guid CLSID_DxcCompiler = new Guid("73e22d93-e6ce-47f3-b5bf-f0664f39c1b0");
        private static Guid CLSID_DxcLinker = new Guid("EF6A8087-B0EA-4D56-9E45-D07E1A8B7806");
        private static Guid CLSID_DxcContainerReflection = new Guid("b9f54489-55b8-400c-ba3a-1675e4728b91");
        private static Guid CLSID_DxcLibrary = new Guid("6245D6AF-66E0-48FD-80B4-4D271796748C");
        private static Guid CLSID_DxcOptimizer = new Guid("AE2CD79F-CC22-453F-9B6B-B124E7A5204C");
        private static Guid CLSID_DxcValidator = new Guid("8CA3E215-F728-4CF3-8CDD-88AF917587A1");

        public static DxcCreateInstanceFn DxcCreateInstanceFn;

        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError =true, CharSet=CharSet.Unicode, ExactSpelling = true)]
        private static extern IntPtr LoadLibraryW([MarshalAs(UnmanagedType.LPWStr)] string fileName);

        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true, CharSet = CharSet.Unicode, ExactSpelling = true)]
        private static extern IntPtr LoadLibraryExW([MarshalAs(UnmanagedType.LPWStr)] string fileName, IntPtr reserved, UInt32 flags);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Ansi, ExactSpelling =true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        private const UInt32 LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR = 0x0100;
        private const UInt32 LOAD_LIBRARY_DEFAULT_DIRS = 0x1000;

        public static DxcCreateInstanceFn LoadDxcCreateInstance(string dllPath, string fnName)
        {
            UInt32 flags = LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_DEFAULT_DIRS;
            IntPtr handle = LoadLibraryExW(dllPath, IntPtr.Zero, flags);
            if (handle == IntPtr.Zero)
            {
                throw new System.ComponentModel.Win32Exception();
            }
            IntPtr fnPtr = GetProcAddress(handle, fnName);
            return (DxcCreateInstanceFn)Marshal.GetDelegateForFunctionPointer(fnPtr, typeof(DxcCreateInstanceFn));
        }

        private static int DxcCreateInstance(
            ref Guid clsid,
            ref Guid iid,
            out object instance)
        {
            return DxcCreateInstanceFn(ref clsid, ref iid, out instance);
        }

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcAssembler CreateDxcAssembler()
        {
            Guid classId = CLSID_DxcAssembler;
            Guid interfaceId = typeof(IDxcAssembler).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcAssembler)result;
        }


        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcCompiler CreateDxcCompiler()
        {
            Guid classId = CLSID_DxcCompiler;
            Guid interfaceId = typeof(IDxcCompiler).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcCompiler)result;
        }

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcLinker CreateDxcLinker()
        {
            Guid classId = CLSID_DxcLinker;
            Guid interfaceId = typeof(IDxcLinker).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcLinker)result;
        }

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcContainerReflection CreateDxcContainerReflection()
        {
            Guid classId = CLSID_DxcContainerReflection;
            Guid interfaceId = typeof(IDxcContainerReflection).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcContainerReflection)result;
        }

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static dia2.IDiaDataSource CreateDxcDiaDataSource()
        {
            Guid classId = CLSID_DxcDiaDataSource;
            Guid interfaceId = typeof(dia2.IDiaDataSource).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (dia2.IDiaDataSource)result;
        }

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcLibrary CreateDxcLibrary()
        {
            Guid classId = CLSID_DxcLibrary;
            Guid interfaceId = typeof(IDxcLibrary).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcLibrary)result;
        }


        // No inlining means that if DxcCreateInstance is not available for any reason, this
        // method will throw an exception (rather than possibly propagating) - caller should
        // guard against failures from this call.
        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcIntelliSense CreateDxcIntelliSense()
        {
            Guid classId = CLSID_DxcIntelliSense;
            Guid interfaceId = typeof(IDxcIntelliSense).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcIntelliSense)result;
        }

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcOptimizer CreateDxcOptimizer()
        {
            Guid classId = CLSID_DxcOptimizer;
            Guid interfaceId = typeof(IDxcOptimizer).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcOptimizer)result;
        }

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcRewriter CreateDxcRewriter()
        {
            Guid classId = CLSID_DxcRewriter;
            Guid interfaceId = typeof(IDxcRewriter).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcRewriter)result;
        }

        [MethodImplAttribute(MethodImplOptions.NoInlining)]
        public static IDxcValidator CreateDxcValidator()
        {
            Guid classId = CLSID_DxcValidator;
            Guid interfaceId = typeof(IDxcValidator).GUID;
            object result;
            DxcCreateInstance(ref classId, ref interfaceId, out result);
            return (IDxcValidator)result;
        }
    }

    [Flags]
    public enum DxcCursorKindFlags : uint
    {
        None = 0,
        Declaration = 0x1,
        Reference = 0x2,
        Expression = 0x4,
        Statement = 0x8,
        Attribute = 0x10,
        Invalid = 0x20,
        TranslationUnit = 0x40,
        Preprocessing = 0x80,
        Unexposed = 0x100,
    }

    /// <summary>
    /// The kind of language construct in a translation unit that a cursor refers to.
    /// </summary>
    public enum DxcCursorKind : uint
    {
        /**
         * \brief A declaration whose specific kind is not exposed via this
         * interface.
         *
         * Unexposed declarations have the same operations as any other kind
         * of declaration; one can extract their location information,
         * spelling, find their definitions, etc. However, the specific kind
         * of the declaration is not reported.
         */
        DxcCursor_UnexposedDecl = 1,
        /** \brief A C or C++ struct. */
        DxcCursor_StructDecl = 2,
        /** \brief A C or C++ union. */
        DxcCursor_UnionDecl = 3,
        /** \brief A C++ class. */
        DxcCursor_ClassDecl = 4,
        /** \brief An enumeration. */
        DxcCursor_EnumDecl = 5,
        /**
         * \brief A field (in C) or non-static data member (in C++) in a
         * struct, union, or C++ class.
         */
        DxcCursor_FieldDecl = 6,
        /** \brief An enumerator constant. */
        DxcCursor_EnumConstantDecl = 7,
        /** \brief A function. */
        DxcCursor_FunctionDecl = 8,
        /** \brief A variable. */
        DxcCursor_VarDecl = 9,
        /** \brief A function or method parameter. */
        DxcCursor_ParmDecl = 10,
        /** \brief An Objective-C \@interface. */
        DxcCursor_ObjCInterfaceDecl = 11,
        /** \brief An Objective-C \@interface for a category. */
        DxcCursor_ObjCCategoryDecl = 12,
        /** \brief An Objective-C \@protocol declaration. */
        DxcCursor_ObjCProtocolDecl = 13,
        /** \brief An Objective-C \@property declaration. */
        DxcCursor_ObjCPropertyDecl = 14,
        /** \brief An Objective-C instance variable. */
        DxcCursor_ObjCIvarDecl = 15,
        /** \brief An Objective-C instance method. */
        DxcCursor_ObjCInstanceMethodDecl = 16,
        /** \brief An Objective-C class method. */
        DxcCursor_ObjCClassMethodDecl = 17,
        /** \brief An Objective-C \@implementation. */
        DxcCursor_ObjCImplementationDecl = 18,
        /** \brief An Objective-C \@implementation for a category. */
        DxcCursor_ObjCCategoryImplDecl = 19,
        /** \brief A typedef */
        DxcCursor_TypedefDecl = 20,
        /** \brief A C++ class method. */
        DxcCursor_CXXMethod = 21,
        /** \brief A C++ namespace. */
        DxcCursor_Namespace = 22,
        /** \brief A linkage specification, e.g. 'extern "C"'. */
        DxcCursor_LinkageSpec = 23,
        /** \brief A C++ constructor. */
        DxcCursor_Constructor = 24,
        /** \brief A C++ destructor. */
        DxcCursor_Destructor = 25,
        /** \brief A C++ conversion function. */
        DxcCursor_ConversionFunction = 26,
        /** \brief A C++ template type parameter. */
        DxcCursor_TemplateTypeParameter = 27,
        /** \brief A C++ non-type template parameter. */
        DxcCursor_NonTypeTemplateParameter = 28,
        /** \brief A C++ template template parameter. */
        DxcCursor_TemplateTemplateParameter = 29,
        /** \brief A C++ function template. */
        DxcCursor_FunctionTemplate = 30,
        /** \brief A C++ class template. */
        DxcCursor_ClassTemplate = 31,
        /** \brief A C++ class template partial specialization. */
        DxcCursor_ClassTemplatePartialSpecialization = 32,
        /** \brief A C++ namespace alias declaration. */
        DxcCursor_NamespaceAlias = 33,
        /** \brief A C++ using directive. */
        DxcCursor_UsingDirective = 34,
        /** \brief A C++ using declaration. */
        DxcCursor_UsingDeclaration = 35,
        /** \brief A C++ alias declaration */
        DxcCursor_TypeAliasDecl = 36,
        /** \brief An Objective-C \@synthesize definition. */
        DxcCursor_ObjCSynthesizeDecl = 37,
        /** \brief An Objective-C \@dynamic definition. */
        DxcCursor_ObjCDynamicDecl = 38,
        /** \brief An access specifier. */
        DxcCursor_CXXAccessSpecifier = 39,

        DxcCursor_FirstDecl = DxcCursor_UnexposedDecl,
        DxcCursor_LastDecl = DxcCursor_CXXAccessSpecifier,

        /* References */
        DxcCursor_FirstRef = 40, /* Decl references */
        DxcCursor_ObjCSuperClassRef = 40,
        DxcCursor_ObjCProtocolRef = 41,
        DxcCursor_ObjCClassRef = 42,
        /**
         * \brief A reference to a type declaration.
         *
         * A type reference occurs anywhere where a type is named but not
         * declared. For example, given:
         *
         * \code
         * typedef unsigned size_type;
         * size_type size;
         * \endcode
         *
         * The typedef is a declaration of size_type (DxcCursor_TypedefDecl),
         * while the type of the variable "size" is referenced. The cursor
         * referenced by the type of size is the typedef for size_type.
         */
        DxcCursor_TypeRef = 43,
        DxcCursor_CXXBaseSpecifier = 44,
        /** 
         * \brief A reference to a class template, function template, template
         * template parameter, or class template partial specialization.
         */
        DxcCursor_TemplateRef = 45,
        /**
         * \brief A reference to a namespace or namespace alias.
         */
        DxcCursor_NamespaceRef = 46,
        /**
         * \brief A reference to a member of a struct, union, or class that occurs in 
         * some non-expression context, e.g., a designated initializer.
         */
        DxcCursor_MemberRef = 47,
        /**
         * \brief A reference to a labeled statement.
         *
         * This cursor kind is used to describe the jump to "start_over" in the 
         * goto statement in the following example:
         *
         * \code
         *   start_over:
         *     ++counter;
         *
         *     goto start_over;
         * \endcode
         *
         * A label reference cursor refers to a label statement.
         */
        DxcCursor_LabelRef = 48,

        /// <summary>
        /// A reference to a set of overloaded functions or function templates
        /// that has not yet been resolved to a specific function or function template.
        /// </summary>
        /// <remarks>
        /// An overloaded declaration reference cursor occurs in C++ templates where
        /// a dependent name refers to a function.
        /// </remarks>
        DxcCursor_OverloadedDeclRef = 49,

        /**
         * \brief A reference to a variable that occurs in some non-expression 
         * context, e.g., a C++ lambda capture list.
         */
        DxcCursor_VariableRef = 50,

        DxcCursor_LastRef = DxcCursor_VariableRef,

        /* Error conditions */
        DxcCursor_FirstInvalid = 70,
        DxcCursor_InvalidFile = 70,
        DxcCursor_NoDeclFound = 71,
        DxcCursor_NotImplemented = 72,
        DxcCursor_InvalidCode = 73,
        DxcCursor_LastInvalid = DxcCursor_InvalidCode,

        /* Expressions */
        DxcCursor_FirstExpr = 100,

        /**
         * \brief An expression whose specific kind is not exposed via this
         * interface.
         *
         * Unexposed expressions have the same operations as any other kind
         * of expression; one can extract their location information,
         * spelling, children, etc. However, the specific kind of the
         * expression is not reported.
         */
        DxcCursor_UnexposedExpr = 100,

        /**
         * \brief An expression that refers to some value declaration, such
         * as a function, varible, or enumerator.
         */
        DxcCursor_DeclRefExpr = 101,

        /**
         * \brief An expression that refers to a member of a struct, union,
         * class, Objective-C class, etc.
         */
        DxcCursor_MemberRefExpr = 102,

        /** \brief An expression that calls a function. */
        DxcCursor_CallExpr = 103,

        /** \brief An expression that sends a message to an Objective-C
         object or class. */
        DxcCursor_ObjCMessageExpr = 104,

        /** \brief An expression that represents a block literal. */
        DxcCursor_BlockExpr = 105,

        /** \brief An integer literal.
         */
        DxcCursor_IntegerLiteral = 106,

        /** \brief A floating point number literal.
         */
        DxcCursor_FloatingLiteral = 107,

        /** \brief An imaginary number literal.
         */
        DxcCursor_ImaginaryLiteral = 108,

        /** \brief A string literal.
         */
        DxcCursor_StringLiteral = 109,

        /** \brief A character literal.
         */
        DxcCursor_CharacterLiteral = 110,

        /** \brief A parenthesized expression, e.g. "(1)".
         *
         * This AST node is only formed if full location information is requested.
         */
        DxcCursor_ParenExpr = 111,

        /** \brief This represents the unary-expression's (except sizeof and
         * alignof).
         */
        DxcCursor_UnaryOperator = 112,

        /** \brief [C99 6.5.2.1] Array Subscripting.
         */
        DxcCursor_ArraySubscriptExpr = 113,

        /** \brief A builtin binary operation expression such as "x + y" or
         * "x <= y".
         */
        DxcCursor_BinaryOperator = 114,

        /** \brief Compound assignment such as "+=".
         */
        DxcCursor_CompoundAssignOperator = 115,

        /** \brief The ?: ternary operator.
         */
        DxcCursor_ConditionalOperator = 116,

        /** \brief An explicit cast in C (C99 6.5.4) or a C-style cast in C++
         * (C++ [expr.cast]), which uses the syntax (Type)expr.
         *
         * For example: (int)f.
         */
        DxcCursor_CStyleCastExpr = 117,

        /** \brief [C99 6.5.2.5]
         */
        DxcCursor_CompoundLiteralExpr = 118,

        /** \brief Describes an C or C++ initializer list.
         */
        DxcCursor_InitListExpr = 119,

        /** \brief The GNU address of label extension, representing &&label.
         */
        DxcCursor_AddrLabelExpr = 120,

        /** \brief This is the GNU Statement Expression extension: ({int X=4; X;})
         */
        DxcCursor_StmtExpr = 121,

        /** \brief Represents a C11 generic selection.
         */
        DxcCursor_GenericSelectionExpr = 122,

        /** \brief Implements the GNU __null extension, which is a name for a null
         * pointer constant that has integral type (e.g., int or long) and is the same
         * size and alignment as a pointer.
         *
         * The __null extension is typically only used by system headers, which define
         * NULL as __null in C++ rather than using 0 (which is an integer that may not
         * match the size of a pointer).
         */
        DxcCursor_GNUNullExpr = 123,

        /** \brief C++'s static_cast<> expression.
         */
        DxcCursor_CXXStaticCastExpr = 124,

        /** \brief C++'s dynamic_cast<> expression.
         */
        DxcCursor_CXXDynamicCastExpr = 125,

        /** \brief C++'s reinterpret_cast<> expression.
         */
        DxcCursor_CXXReinterpretCastExpr = 126,

        /** \brief C++'s const_cast<> expression.
         */
        DxcCursor_CXXConstCastExpr = 127,

        /** \brief Represents an explicit C++ type conversion that uses "functional"
         * notion (C++ [expr.type.conv]).
         *
         * Example:
         * \code
         *   x = int(0.5);
         * \endcode
         */
        DxcCursor_CXXFunctionalCastExpr = 128,

        /** \brief A C++ typeid expression (C++ [expr.typeid]).
         */
        DxcCursor_CXXTypeidExpr = 129,

        /** \brief [C++ 2.13.5] C++ Boolean Literal.
         */
        DxcCursor_CXXBoolLiteralExpr = 130,

        /** \brief [C++0x 2.14.7] C++ Pointer Literal.
         */
        DxcCursor_CXXNullPtrLiteralExpr = 131,

        /** \brief Represents the "this" expression in C++
         */
        DxcCursor_CXXThisExpr = 132,

        /** \brief [C++ 15] C++ Throw Expression.
         *
         * This handles 'throw' and 'throw' assignment-expression. When
         * assignment-expression isn't present, Op will be null.
         */
        DxcCursor_CXXThrowExpr = 133,

        /** \brief A new expression for memory allocation and constructor calls, e.g:
         * "new CXXNewExpr(foo)".
         */
        DxcCursor_CXXNewExpr = 134,

        /** \brief A delete expression for memory deallocation and destructor calls,
         * e.g. "delete[] pArray".
         */
        DxcCursor_CXXDeleteExpr = 135,

        /** \brief A unary expression.
         */
        DxcCursor_UnaryExpr = 136,

        /** \brief An Objective-C string literal i.e. @"foo".
         */
        DxcCursor_ObjCStringLiteral = 137,

        /** \brief An Objective-C \@encode expression.
         */
        DxcCursor_ObjCEncodeExpr = 138,

        /** \brief An Objective-C \@selector expression.
         */
        DxcCursor_ObjCSelectorExpr = 139,

        /** \brief An Objective-C \@protocol expression.
         */
        DxcCursor_ObjCProtocolExpr = 140,

        /** \brief An Objective-C "bridged" cast expression, which casts between
         * Objective-C pointers and C pointers, transferring ownership in the process.
         *
         * \code
         *   NSString *str = (__bridge_transfer NSString *)CFCreateString();
         * \endcode
         */
        DxcCursor_ObjCBridgedCastExpr = 141,

        /** \brief Represents a C++0x pack expansion that produces a sequence of
         * expressions.
         *
         * A pack expansion expression contains a pattern (which itself is an
         * expression) followed by an ellipsis. For example:
         *
         * \code
         * template<typename F, typename ...Types>
         * void forward(F f, Types &&...args) {
         *  f(static_cast<Types&&>(args)...);
         * }
         * \endcode
         */
        DxcCursor_PackExpansionExpr = 142,

        /** \brief Represents an expression that computes the length of a parameter
         * pack.
         *
         * \code
         * template<typename ...Types>
         * struct count {
         *   static const unsigned value = sizeof...(Types);
         * };
         * \endcode
         */
        DxcCursor_SizeOfPackExpr = 143,

        /* \brief Represents a C++ lambda expression that produces a local function
         * object.
         *
         * \code
         * void abssort(float *x, unsigned N) {
         *   std::sort(x, x + N,
         *             [](float a, float b) {
         *               return std::abs(a) < std::abs(b);
         *             });
         * }
         * \endcode
         */
        DxcCursor_LambdaExpr = 144,

        /** \brief Objective-c Boolean Literal.
         */
        DxcCursor_ObjCBoolLiteralExpr = 145,

        /** \brief Represents the "self" expression in a ObjC method.
         */
        DxcCursor_ObjCSelfExpr = 146,

        DxcCursor_LastExpr = DxcCursor_ObjCSelfExpr,

        /* Statements */
        DxcCursor_FirstStmt = 200,
        /**
         * \brief A statement whose specific kind is not exposed via this
         * interface.
         *
         * Unexposed statements have the same operations as any other kind of
         * statement; one can extract their location information, spelling,
         * children, etc. However, the specific kind of the statement is not
         * reported.
         */
        DxcCursor_UnexposedStmt = 200,

        /** \brief A labelled statement in a function. 
         *
         * This cursor kind is used to describe the "start_over:" label statement in 
         * the following example:
         *
         * \code
         *   start_over:
         *     ++counter;
         * \endcode
         *
         */
        DxcCursor_LabelStmt = 201,

        /** \brief A group of statements like { stmt stmt }.
         *
         * This cursor kind is used to describe compound statements, e.g. function
         * bodies.
         */
        DxcCursor_CompoundStmt = 202,

        /** \brief A case statement.
         */
        DxcCursor_CaseStmt = 203,

        /** \brief A default statement.
         */
        DxcCursor_DefaultStmt = 204,

        /** \brief An if statement
         */
        DxcCursor_IfStmt = 205,

        /** \brief A switch statement.
         */
        DxcCursor_SwitchStmt = 206,

        /** \brief A while statement.
         */
        DxcCursor_WhileStmt = 207,

        /** \brief A do statement.
         */
        DxcCursor_DoStmt = 208,

        /** \brief A for statement.
         */
        DxcCursor_ForStmt = 209,

        /** \brief A goto statement.
         */
        DxcCursor_GotoStmt = 210,

        /** \brief An indirect goto statement.
         */
        DxcCursor_IndirectGotoStmt = 211,

        /** \brief A continue statement.
         */
        DxcCursor_ContinueStmt = 212,

        /** \brief A break statement.
         */
        DxcCursor_BreakStmt = 213,

        /** \brief A return statement.
         */
        DxcCursor_ReturnStmt = 214,

        /** \brief A GCC inline assembly statement extension.
         */
        DxcCursor_GCCAsmStmt = 215,
        DxcCursor_AsmStmt = DxcCursor_GCCAsmStmt,

        /** \brief Objective-C's overall \@try-\@catch-\@finally statement.
         */
        DxcCursor_ObjCAtTryStmt = 216,

        /** \brief Objective-C's \@catch statement.
         */
        DxcCursor_ObjCAtCatchStmt = 217,

        /** \brief Objective-C's \@finally statement.
         */
        DxcCursor_ObjCAtFinallyStmt = 218,

        /** \brief Objective-C's \@throw statement.
         */
        DxcCursor_ObjCAtThrowStmt = 219,

        /** \brief Objective-C's \@synchronized statement.
         */
        DxcCursor_ObjCAtSynchronizedStmt = 220,

        /** \brief Objective-C's autorelease pool statement.
         */
        DxcCursor_ObjCAutoreleasePoolStmt = 221,

        /** \brief Objective-C's collection statement.
         */
        DxcCursor_ObjCForCollectionStmt = 222,

        /** \brief C++'s catch statement.
         */
        DxcCursor_CXXCatchStmt = 223,

        /** \brief C++'s try statement.
         */
        DxcCursor_CXXTryStmt = 224,

        /** \brief C++'s for (* : *) statement.
         */
        DxcCursor_CXXForRangeStmt = 225,

        /** \brief Windows Structured Exception Handling's try statement.
         */
        DxcCursor_SEHTryStmt = 226,

        /** \brief Windows Structured Exception Handling's except statement.
         */
        DxcCursor_SEHExceptStmt = 227,

        /** \brief Windows Structured Exception Handling's finally statement.
         */
        DxcCursor_SEHFinallyStmt = 228,

        /** \brief A MS inline assembly statement extension.
         */
        DxcCursor_MSAsmStmt = 229,

        /** \brief The null satement ";": C99 6.8.3p3.
         *
         * This cursor kind is used to describe the null statement.
         */
        DxcCursor_NullStmt = 230,

        /** \brief Adaptor class for mixing declarations with statements and
         * expressions.
         */
        DxcCursor_DeclStmt = 231,

        /** \brief OpenMP parallel directive.
         */
        DxcCursor_OMPParallelDirective = 232,

        DxcCursor_LastStmt = DxcCursor_OMPParallelDirective,

        /**
         * \brief Cursor that represents the translation unit itself.
         *
         * The translation unit cursor exists primarily to act as the root
         * cursor for traversing the contents of a translation unit.
         */
        DxcCursor_TranslationUnit = 300,

        /* Attributes */
        DxcCursor_FirstAttr = 400,
        /**
         * \brief An attribute whose specific kind is not exposed via this
         * interface.
         */
        DxcCursor_UnexposedAttr = 400,

        DxcCursor_IBActionAttr = 401,
        DxcCursor_IBOutletAttr = 402,
        DxcCursor_IBOutletCollectionAttr = 403,
        DxcCursor_CXXFinalAttr = 404,
        DxcCursor_CXXOverrideAttr = 405,
        DxcCursor_AnnotateAttr = 406,
        DxcCursor_AsmLabelAttr = 407,
        DxcCursor_PackedAttr = 408,
        DxcCursor_LastAttr = DxcCursor_PackedAttr,

        /* Preprocessing */
        DxcCursor_PreprocessingDirective = 500,
        DxcCursor_MacroDefinition = 501,
        DxcCursor_MacroExpansion = 502,
        DxcCursor_MacroInstantiation = DxcCursor_MacroExpansion,
        DxcCursor_InclusionDirective = 503,
        DxcCursor_FirstPreprocessing = DxcCursor_PreprocessingDirective,
        DxcCursor_LastPreprocessing = DxcCursor_InclusionDirective,

        /* Extra Declarations */
        /**
         * \brief A module import declaration.
         */
        DxcCursor_ModuleImportDecl = 600,
        DxcCursor_FirstExtraDecl = DxcCursor_ModuleImportDecl,
        DxcCursor_LastExtraDecl = DxcCursor_ModuleImportDecl
    };

    /// <summary>Describes a kind of token.</summary>
    public enum DxcTokenKind : uint
    {
        /// <summary>A token that contains some kind of punctuation.</summary>
        Punctuation = 0,

        /// <summary>A language keyword.</summary>
        Keyword = 1,

        /// <summary>An identifier (that is not a keyword).</summary>
        Identifier = 2,

        /// <summary>A numeric, string, or character literal.</summary>
        Literal = 3,

        /// <summary>A comment.</summary>
        Comment = 4,

        /// <summary>An unknown token (possibly known to a future version).</summary>
        Unknown = 5,

        /// <summary>The token matches a built-in type.</summary>
        BuiltInType = 6,
    };

    public enum DxcDiagnosticDisplayOptions : uint
    {
        // Display the source-location information where the diagnostic was located.
        DxcDiagnostic_DisplaySourceLocation = 0x01,

        // If displaying the source-location information of the diagnostic,
        // also include the column number.
        DxcDiagnostic_DisplayColumn = 0x02,

        // If displaying the source-location information of the diagnostic,
        // also include information about source ranges in a machine-parsable format.
        DxcDiagnostic_DisplaySourceRanges = 0x04,

        // Display the option name associated with this diagnostic, if any.
        DxcDiagnostic_DisplayOption = 0x08,

        // Display the category number associated with this diagnostic, if any.
        DxcDiagnostic_DisplayCategoryId = 0x10,

        // Display the category name associated with this diagnostic, if any.
        DxcDiagnostic_DisplayCategoryName = 0x20
    };

    public enum DxcDiagnosticSeverity
    {
        // A diagnostic that has been suppressed, e.g., by a command-line option.
        DxcDiagnostic_Ignored = 0,

        // This diagnostic is a note that should be attached to the previous (non-note) diagnostic.
        DxcDiagnostic_Note = 1,

        // This diagnostic indicates suspicious code that may not be wrong.
        DxcDiagnostic_Warning = 2,

        // This diagnostic indicates that the code is ill-formed.
        DxcDiagnostic_Error = 3,

        // This diagnostic indicates that the code is ill-formed such that future
        // parser rec unlikely to produce useful results.
        DxcDiagnostic_Fatal = 4
    };

    public enum DxcTranslationUnitFlags : uint
    {
        // Used to indicate that no special translation-unit options are needed.
        DxcTranslationUnitFlags_None = 0x0,

        // Used to indicate that the parser should construct a "detailed"
        // preprocessing record, including all macro definitions and instantiations.
        DxcTranslationUnitFlags_DetailedPreprocessingRecord = 0x01,

        // Used to indicate that the translation unit is incomplete.
        DxcTranslationUnitFlags_Incomplete = 0x02,

        // Used to indicate that the translation unit should be built with an
        // implicit precompiled header for the preamble.
        DxcTranslationUnitFlags_PrecompiledPreamble = 0x04,

        // Used to indicate that the translation unit should cache some
        // code-completion results with each reparse of the source file.
        DxcTranslationUnitFlags_CacheCompletionResults = 0x08,

        // Used to indicate that the translation unit will be serialized with
        // SaveTranslationUnit.
        DxcTranslationUnitFlags_ForSerialization = 0x10,

        // DEPRECATED
        DxcTranslationUnitFlags_CXXChainedPCH = 0x20,

        // Used to indicate that function/method bodies should be skipped while parsing.
        DxcTranslationUnitFlags_SkipFunctionBodies = 0x40,

        // Used to indicate that brief documentation comments should be
        // included into the set of code completions returned from this translation
        // unit.
        DxcTranslationUnitFlags_IncludeBriefCommentsInCodeCompletion = 0x80,

        // Used to indicate that compilation should occur on the caller's thread.
        DxcTranslationUnitFlags_UseCallerThread = 0x800,
    };

    [ComImport]
    [Guid("8BA5FB08-5195-40e2-AC58-0D989C3A0102")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcBlob
    {
        [PreserveSig]
        unsafe char* GetBufferPointer();
        [PreserveSig]
        UInt32 GetBufferSize();
    }

    [ComImport]
    [Guid("8BA5FB08-5195-40e2-AC58-0D989C3A0102")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcBlobEncoding : IDxcBlob
    {
        System.UInt32 GetEncoding(out bool unknown, out UInt32 codePage);
    }

    [ComImport]
    [Guid("CEDB484A-D4E9-445A-B991-CA21CA157DC2")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcOperationResult
    {
        Int32 GetStatus();
        IDxcBlob GetResult();
        IDxcBlobEncoding GetErrors();
    }
    [ComImport]
    [Guid("7f61fc7d-950d-467f-b3e3-3c02fb49187c")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcIncludeHandler
    {
        IDxcBlob LoadSource(string fileName);
    }

    [ComImport]
    [Guid("091f7a26-1c1f-4948-904b-e6e3a8a771d5")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcAssembler
    {
        IDxcOperationResult AssembleToContainer(IDxcBlob source);
    }


    [ComImport]
    [Guid("8c210bf3-011f-4422-8d70-6f9acb8db617")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcCompiler
    {
        IDxcOperationResult Compile(IDxcBlob source, string sourceName, string entryPoint, string targetProfile,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType =UnmanagedType.LPWStr)]
            string[] arguments,
            int argCount, DXCDefine[] defines, int defineCount, IDxcIncludeHandler includeHandler);
        IDxcOperationResult Preprocess(IDxcBlob source, string sourceName, string[] arguments, int argCount,
            DXCDefine[] defines, int defineCount, IDxcIncludeHandler includeHandler);
        IDxcBlobEncoding Disassemble(IDxcBlob source);
    }

    [ComImport]
    [Guid("d2c21b26-8350-4bdc-976a-331ce6f4c54c")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcContainerReflection
    {
        void Load(IDxcBlob container);
        uint GetPartCount();
        uint GetPartKind(uint idx);
        IDxcBlob GetPartContent(uint idx);
        [PreserveSig]
        int FindFirstPartKind(uint kind, out uint result);
        [return: MarshalAs(UnmanagedType.Interface, IidParameterIndex = 1)]
        object GetPartReflection(uint idx, Guid iid);
    }

    [ComImport]
    [Guid("e5204dc7-d18c-4c3c-bdfb-851673980fe7")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcLibrary
    {
        void SetMalloc(object malloc);
        IDxcBlob CreateBlobFromBlob(IDxcBlob blob, UInt32 offset, UInt32 length);
        IDxcBlobEncoding CreateBlobFromFile(string fileName, System.IntPtr codePage);
        IDxcBlobEncoding CreateBlobWithEncodingFromPinned(byte[] text, UInt32 size, UInt32 codePage);
        // IDxcBlobEncoding CreateBlobWithEncodingOnHeapCopy(IntrPtr text, UInt32 size, UInt32 codePage);
        IDxcBlobEncoding CreateBlobWithEncodingOnHeapCopy(string text, UInt32 size, UInt32 codePage);
        IDxcBlobEncoding CreateBlobWithEncodingOnMalloc(string text, object malloc, UInt32 size, UInt32 codePage);
        IDxcIncludeHandler CreateIncludeHandler();
        System.Runtime.InteropServices.ComTypes.IStream CreateStreamFromBlobReadOnly(IDxcBlob blob);
        IDxcBlobEncoding GetBlobAstUf8(IDxcBlob blob);
        IDxcBlobEncoding GetBlobAstUf16(IDxcBlob blob);
    }

    [ComImport]
    [Guid("F1B5BE2A-62DD-4327-A1C2-42AC1E1E78E6")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcLinker : IDxcCompiler
    {
        // Register a library with name to ref it later.
        int RegisterLibrary(string libName, IDxcBlob library);

        int Link(
            string entryName,
            string targetProfile,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType =UnmanagedType.LPWStr)]
            string[] libNames,
            int libCount,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType =UnmanagedType.LPWStr)]
            string[] pArguments,
            int argCount,
            out IDxcOperationResult result
            );
    }

    [ComImport]
    [Guid("AE2CD79F-CC22-453F-9B6B-B124E7A5204C")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcOptimizerPass
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetOptionName();
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetDescription();
        uint GetOptionArgCount();
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetOptionArgName(uint argIndex);
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetOptionArgDescription(uint argIndex);
    }

    [ComImport]
    [Guid("25740E2E-9CBA-401B-9119-4FB42F39F270")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IDxcOptimizer
    {
        int GetAvailablePassCount();
        IDxcOptimizerPass GetAvailablePass(int index);

        void RunOptimizer(IDxcBlob source,
            [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr)] string[] options, int optionCount, 
            out IDxcBlob outputModule, out IDxcBlobEncoding outputText);
    }
}
