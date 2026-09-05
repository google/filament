///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// AsmColorizer.cs                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides a component that provides color information for DXIL assembler.  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Text;

using DotNetDxc;

namespace MainNs
{
    enum AsmRangeKind
    {
        WS,
        Comment,
        LLVMTypeName,
        Keyword,
        Metadata,
        Punctuation,
        Instruction,
        Label,
        GlobalVar,
        LocalVar,
        Number,
        StringConstant,
        AttributeGroup,
        Other
    }

    struct AsmRange
    {
        public AsmRange(int start, int length, AsmRangeKind rangeKind)
        {
            this.Start = start;
            this.Length = length;
            this.RangeKind = rangeKind;
        }
        public int Start;
        public int Length;
        public AsmRangeKind RangeKind;
    }

    /// <summary>
    /// Simplified colorizer. Takes some shortcuts assuming well-formed assembler in some cases.
    /// </summary>
    /// <remarks>
    /// See lib\AsmParser\LLToken.h for more details.
    /// </remarks>
    class AsmColorizer
    {
        private static readonly string Punctuation = ".,(){}\\\"";
        private static readonly string WS = " \t\r\n";
        private static readonly string[] InstKeywords;
        private static readonly string[] TypeKeywords;
        private static readonly string[] Keywords;
        private static bool IsAlpha(char ch)
        {
            return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
        }
        private static bool IsAlnum(char ch)
        {
            return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9');
        }
        private static bool IsMetadataChar(char ch)
        {
            // digits can't be leads, but already identified by leading '$'
            return IsAlnum(ch) || ch == '-' || ch == '$' || ch == '.' || ch == '_' || ch == '\\';
        }
        private static bool IsNumericLead(char ch)
        {
            return ('0' <= ch && ch <= '9') || ch == '-' || ch == '+';
        }
        private static bool IsNumeric(char ch)
        {
            return ch == 'E' || ch == 'e' || ch == '.' || IsNumericLead(ch);
        }
        private static bool IsLabelChar(char ch)
        {
            return IsAlnum(ch) || ch == '-' || ch == '$' || ch == '.' || ch == '_';
        }
        private static bool IsWS(char ch)
        {
            return WS.IndexOf(ch) >= 0;
        }
        private static bool IsPunctuation(char ch)
        {
            return Punctuation.IndexOf(ch) >= 0;
        }
        private static AsmRangeKind ClassifyIdentifierLike(string text, int start, int end)
        {
            char last = text[end - 1];
            if (last == ':')
                return AsmRangeKind.Label;

            int intWidth;
            if (text[start] == 'i' && Int32.TryParse(text.Substring(start + 1, 1), out intWidth))
                return AsmRangeKind.LLVMTypeName;

            string val = text.Substring(start, end - start);
            if (Array.BinarySearch(Keywords, val, StringComparer.Ordinal) >= 0)
                return AsmRangeKind.Keyword;
            if (Array.BinarySearch(TypeKeywords, val, StringComparer.Ordinal) >= 0)
                return AsmRangeKind.LLVMTypeName;
            if (Array.BinarySearch(InstKeywords, val, StringComparer.Ordinal) >= 0)
                return AsmRangeKind.Instruction;
            // Pending: DWKEYWORD, a few others
            return AsmRangeKind.Other;
        }
        static AsmColorizer()
        {
            string[] keywords = (
                "true,false,declare,define,global,constant," +
                "private,internal,available_externally,linkonce,linkonce_odr,weak,weak_odr,appending,dllimport,dllexport," +
                "common,default,hidden,protected,unnamed_addr,externally_initialized,extern_weak,external,thread_local,localdynamic,initialexec,localexec,zeroinitializer,undef," +
                "null,to,tail,musttail,target,triple,unwind,deplibs,datalayout,volatile,atomic,unordered,monotonic,acquire,release,acq_rel,seq_cst,singlethread," +
                "nnan,ninf,nsz,arcp,fast,nuw,nsw,exact,inbounds,align,addrspace,section,alias,module,asm,sideeffect,alignstack,inteldialect,gc,prefix,prologue," +
                "null,to,tail,musttail,target,triple,unwind,deplibs,datalayout,volatile,atomic,unordered,monotonic,acquire,release,acq_rel,seq_cst,singlethread," +
                "ccc,fastcc,coldcc,x86_stdcallcc,x86_fastcallcc,x86_thiscallcc,x86_vectorcallcc,arm_apcscc,arm_aapcscc,arm_aapcs_vfpcc,msp430_intrcc," +
                "ptx_kernel,ptx_device,spir_kernel,spir_func,intel_ocl_bicc,x86_64_sysvcc,x86_64_win64cc,webkit_jscc,anyregcc," +
                "preserve_mostcc,preserve_allcc,ghccc,cc,c," +
                "attributes," +
                "alwaysinline,argmemonly,builtin,byval,inalloca,cold,convergent,dereferenceable,dereferenceable_or_null,inlinehint,inreg,jumptable," +
                "minsize,naked,nest,noalias,nobuiltin,nocapture,noduplicate,noimplicitfloat,noinline,nonlazybind,nonnull,noredzone,noreturn," +
                "nounwind,optnone,optsize,readnone,readonly,returned,returns_twice,signext,sret,ssp,sspreq,sspstrong,safestack," +
                "sanitize_address,sanitize_thread,sanitize_memory,uwtable,zeroext," +
                "type,opaque," +
                "comdat," +
                "any,exactmatch,largest,noduplicates,samesize," + // Comdat types
                "eq,ne,slt,sgt,sle,sge,ult,ugt,ule,uge,oeq,one,olt,ogt,ole,oge,ord,uno,ueq,une," +
                "xchg,nand,max,min,umax,umin," +
                "x,blockaddress," +
                "distinct," + // Metadata types.
                "uselistorder,uselistorder_bb," + // Use-list order directives.
                "personality,cleanup,catch,filter"
                ).Split(',');

            Array.Sort(keywords, StringComparer.Ordinal);
            Keywords = keywords;

            string[] typeKeywords = "void,half,float,double,x86_fp80,fp128,ppc_fp128,label,metadata,x86_mmx".Split(',');
            Array.Sort(typeKeywords, StringComparer.Ordinal);
            TypeKeywords = typeKeywords;

            string[] instKeywords = (
              "add,fadd,sub,fsub,mul,fmul,udiv,sdiv,fdiv,urem,srem,frem,shl,lshr,ashr,and,or,xor,icmp,fcmp," +
              "phi,call,trunc,zext,sext,fptrunc,fpext,uitofp,sitofp,fptoui,fptosi,inttoptr,ptrtoint,bitcast,addrspacecast,select,va,ret,br,switch,indirectbr,invoke,resume,unreachable," +
              "alloca,load,store,cmpxchg,atomicrmw,fence,getelementptr," +
              "extractelement,insertelement,shufflevector,extractvalue,insertvalue,landingpad"
            ).Split(',');
            Array.Sort(instKeywords, StringComparer.Ordinal);
            InstKeywords = instKeywords;
        }

        private static void LexVar(string text, int start, ref int end)
        {
            char ch = text[start];
            if (ch == '"')
            {
                for (;;)
                {
                    if (end == text.Length) return;
                    if (text[end] == '"') return; // should unescape here
                    ++end;
                }
            }
            for (;;)
            {
                if (end == text.Length) return;
                if (!IsLabelChar(text[end])) return;
                ++end;
            }
        }

        internal IEnumerable<AsmRange> GetColorRanges(string text)
        {
            AsmRange range = new AsmRange();
            while (GetNextRange(text, ref range))
            {
                yield return range;
            }
        }

        internal IEnumerable<AsmRange> GetColorRanges(string text, int start, int end)
        {
            AsmRange range = new AsmRange();
            range.Start = start;
            while (range.Start < end && GetNextRange(text, ref range))
            {
                yield return range;
            }
        }

        internal bool GetNextRange(string text, ref AsmRange range)
        {
            int start, end;
            start = range.Start + range.Length;
            while (start != text.Length)
            {
                end = start + 1;
                AsmRangeKind kind = AsmRangeKind.Other;
                char ch = text[start];
                if (IsWS(ch))
                {
                    kind = AsmRangeKind.WS;
                    while (end != text.Length && IsWS(text[end]))
                        ++end;
                }
                else if (ch == ';')
                {
                    kind = AsmRangeKind.Comment;
                    while (end != text.Length && text[end] != '\n')
                        ++end;
                    if (end != text.Length) ++end;
                }
                else if (ch == '@')
                {
                    kind = AsmRangeKind.GlobalVar;
                    LexVar(text, start, ref end);
                }
                //else if (ch == '$')
                //{
                    // lex dollar
                //}
                else if (ch == '%')
                {
                    kind = AsmRangeKind.LocalVar;
                    LexVar(text, start, ref end);
                }
                else if (ch == '"')
                {
                    kind = AsmRangeKind.StringConstant;
                    while (end != text.Length && (text[end] != '"'))
                        ++end;
                    if (end != text.Length) ++end;
                    if (end != text.Length && text[end] == ':')
                    {
                        ++end;
                        kind = AsmRangeKind.Label;
                    }
                }
                else if (ch == '!')
                {
                    kind = AsmRangeKind.Metadata;
                    while (end != text.Length && IsMetadataChar(text[end]))
                        ++end;
                }
                else if (ch == '#')
                {
                    kind = AsmRangeKind.AttributeGroup;
                    while (end != text.Length && Char.IsDigit(text[end]))
                        ++end;
                    if (end != text.Length) ++end;
                }
                else if (IsNumericLead(ch))
                {
                    // lex number or positive
                    kind = AsmRangeKind.Number;
                    while (end != text.Length && IsNumeric(text[end]))
                        ++end;
                    if (end != text.Length) ++end;
                }
                else if (IsPunctuation(ch))
                {
                    kind = AsmRangeKind.Punctuation;
                }
                else
                {
                    kind = AsmRangeKind.Other;
                    while (end != text.Length && !IsWS(text[end]) && !IsPunctuation(text[end]))
                        ++end;
                    kind = ClassifyIdentifierLike(text, start, end);
                }
                range = new AsmRange(start, end - start, kind);
                return true;
            }
            return false;
        }
    }
}
