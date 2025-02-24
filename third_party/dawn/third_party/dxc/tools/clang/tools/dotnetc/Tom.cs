///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Tom.cs                                                                    //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using System;
using System.Runtime.InteropServices;

namespace Tom
{
    [ComImport, Guid("8CC497C0-A1DF-11ce-8098-00AA0047BE5D"), InterfaceType(ComInterfaceType.InterfaceIsDual)]
    internal interface ITextDocument
    {
        void Placeholder_Name();
        ITextSelection Selection
        {
            [return: MarshalAs(UnmanagedType.Interface)]
            get;
        }
        void Placeholder_StoryCount();
        void Placeholder_StoryRanges();
        void Placeholder_get_Saved();
        void Placeholder_set_Saved();
        void Placeholder_get_DefaultTabStop();
        void Placeholder_set_DefaultTabStop();
        void Placeholder_New();
        void Placeholder_Open();
        void Placeholder_Save();
        void Placeholder_Freeze();
        void Placeholder_Unfreeze();
        void Placeholder_BeginEditCollection();
        void Placeholder_EndEditCollection();
        void Placeholder_Undo();
        void Placeholder_Redo();
        [return: MarshalAs(UnmanagedType.Interface)]
        ITextRange Range(int cp1, int cp2);
        [return: MarshalAs(UnmanagedType.Interface)]
        ITextRange RangeFromPoint(int x, int y);
    }

    [ComImport, Guid("8CC497C2-A1DF-11ce-8098-00AA0047BE5D"), InterfaceType(ComInterfaceType.InterfaceIsDual)]
    internal interface ITextRange
    {
        string Text { get; }
        void Placeholder_set_Text();
        void Placeholder_get_Char();
        void Placeholder_set_Char();
        //void GetDuplicate([MarshalAs(UnmanagedType.Interface)]out ITextRange ppRange);
        [return: MarshalAs(UnmanagedType.Interface)]
        ITextRange GetDuplicate();
        void Placeholder_get_FormattedText();
        void Placeholder_set_FormattedText();
        int Start { get; set; }
        int End { get; set; }
        ITextFont Font { get; set; }
        ITextPara Para { get; set; }
        int StoryLength { get; }
        TomStory StoryType { get; }
        void Collapse(TomStartEnd bStart);
        int Expand(TomUnit unit);
        void Placeholder_GetIndex();
        void Placeholder_SetIndex();
        void SetRange(int cp1, int cp2);
        TomBool InRange(ITextRange range);
        void Placeholder_InStory();
        TomBool IsEqual(ITextRange range);
        void Select();
        int StartOf(int type, int extend);
        int EndOf(TomUnit unit, TomExtend extend);
        int Move(TomUnit unit, int count);
        int MoveStart(TomUnit unit, int count);
        int MoveEnd(TomUnit unit, int count);
        void Placeholder_MoveWhile();
        void Placeholder_MoveStartWhile();
        void Placeholder_MoveEndWhile();
        void Placeholder_MoveUntil();
        void Placeholder_MoveStartUntil();
        void Placeholder_MoveEndUntil();
        int FindText(string bstr, int count, TomMatch flags);
        int FindTextStart(string bstr, int count, TomMatch flags);
        int FindTextEnd(string bstr, int count, TomMatch flags);
        void Placeholder_Delete();
        void Placeholder_Cut();
        void Placeholder_Copy();
        void Placeholder_Paste();
        void Placeholder_CanPaste();
        void Placeholder_CanEdit();
        void Placeholder_ChangeCase();
        [PreserveSig]
        int GetPoint(TomGetPoint type, out int px, out int py);
        void Placeholder_SetPoint();
        void ScrollIntoView(TomStartEnd scrollvalue);
        [PreserveSig]
        int GetEmbeddedObject([MarshalAs(UnmanagedType.IUnknown)]out object ppObj);
    }

    [ComImport, Guid("8CC497C1-A1DF-11ce-8098-00AA0047BE5D"), InterfaceType(ComInterfaceType.InterfaceIsDual)]
    internal interface ITextSelection : ITextRange
    {
        #region ITextRange
        new string Text { get; }
        new void Placeholder_set_Text();
        new void Placeholder_get_Char();
        new void Placeholder_set_Char();
        //new void GetDuplicate([MarshalAs(UnmanagedType.Interface)]out ITextRange ppRange);
        [return: MarshalAs(UnmanagedType.Interface)]
        new ITextRange GetDuplicate();
        new void Placeholder_get_FormattedText();
        new void Placeholder_set_FormattedText();
        new int Start { get; set; }
        new int End { get; set; }
        new ITextFont Font { get; set; }
        new ITextPara Para { get; set; }
        new int StoryLength { get; }
        new TomStory StoryType { get; }
        new void Collapse(TomStartEnd bStart);
        new int Expand(TomUnit unit);
        new void Placeholder_GetIndex();
        new void Placeholder_SetIndex();
        new void SetRange(int cp1, int cp2);
        new TomBool InRange(ITextRange range);
        new void Placeholder_InStory();
        new TomBool IsEqual(ITextRange range);
        new void Select();
        new int StartOf(int type, int extend);
        new int EndOf(TomUnit unit, TomExtend extend);
        new int Move(TomUnit unit, int count);
        new int MoveStart(TomUnit unit, int count);
        new int MoveEnd(TomUnit unit, int count);
        new void Placeholder_MoveWhile();
        new void Placeholder_MoveStartWhile();
        new void Placeholder_MoveEndWhile();
        new void Placeholder_MoveUntil();
        new void Placeholder_MoveStartUntil();
        new void Placeholder_MoveEndUntil();
        new int FindText(string bstr, int count, TomMatch flags);
        new int FindTextStart(string bstr, int count, TomMatch flags);
        new int FindTextEnd(string bstr, int count, TomMatch flags);
        new void Placeholder_Delete();
        new void Placeholder_Cut();
        new void Placeholder_Copy();
        new void Placeholder_Paste();
        new void Placeholder_CanPaste();
        new void Placeholder_CanEdit();
        new void Placeholder_ChangeCase();
        [PreserveSig]
        new int GetPoint(TomGetPoint type, out int px, out int py);
        new void Placeholder_SetPoint();
        new void ScrollIntoView(TomStartEnd scrollvalue);
        [PreserveSig]
        new int GetEmbeddedObject([MarshalAs(UnmanagedType.IUnknown)]out object ppObj);
        #endregion ITextRange
        TomSelectionFlags Flags { get; set; }
        void Placeholder_Type();
        void Placeholder_MoveLeft();
        void Placeholder_MoveRight();
        void Placeholder_MoveUp();
        void Placeholder_MoveDown();
        void Placeholder_HomeKey();
        void Placeholder_EndKey();
        void Placeholder_TypeText();
    }

    [ComImport, Guid("8CC497C3-A1DF-11ce-8098-00AA0047BE5D"), InterfaceType(ComInterfaceType.InterfaceIsDual)]
    internal interface ITextFont
    {
        //[propget]GetDuplicate([retval][out] ITextFont **ppFont)
        ITextFont GetDuplicate();
        //[propput]SetDuplicate([in] ITextFont *pFont)
        void Placeholder_SetDuplicate();
        //CanChange([retval][out] long *pB)
        void Placeholder_CanChange();
        //IsEqual([in] ITextFont *pFont, [retval][out] long *pB)
        void Placeholder_IsEqual();
        //Reset([in] long Value)
        void Placeholder_Reset();
        //[propget]GetStyle([retval][out] long *pValue)
        void Placeholder_GetStyle();
        //[propput]SetStyle([in] long Value)
        void Placeholder_SetStyle();
        //[propget]GetAllCaps([retval][out] long *pValue)
        //[propput]SetAllCaps([in] long Value)
        TomBool AllCaps { get; set; }
        //[propget]GetAnimation([retval][out] long *pValue)
        //[propput]SetAnimation([in] long Value)
        TomAnimation Animation { get; set; }
        //[propget]GetBackColor([retval][out] long *pValue)
        //[propput]SetBackColor([in] long Value)
        // if high byte is zero then value is COLORREF.
        // if high byte is one then PALETTEINDEX.
        int BackColor { get; set; }
        //[propget]GetBold([retval][out] long *pValue)
        void Placeholder_GetBold();
        //[propput]SetBold([in] long Value)
        void Placeholder_SetBold();
        //[propget]GetEmboss([retval][out] long *pValue)
        //[propput]SetEmboss([in] long Value)
        TomBool Emboss { get; set; }
        //[propget]GetForeColor([retval][out] long *pValue)
        //[propput]SetForeColor([in] long Value)
        int ForeColor { get; set; }
        //[propget]GetHidden([retval][out] long *pValue)
        //[propput]SetHidden([in] long Value)
        TomBool Hidden { get; set; }
        //[propget]GetEngrave([retval][out] long *pValue)
        //[propput]SetEngrave([in] long Value)
        TomBool Engrave { get; set; }
        //[propget]GetItalic([retval][out] long *pValue)
        //[propput]SetItalic([in] long Value)
        TomBool Italic { get; set; }
        //[propget]GetKerning([retval][out] float *pValue)
        void Placeholder_GetKerning();
        //[propput]SetKerning([in] float Value)
        void Placeholder_SetKerning();
        //[propget]GetLanguageID([retval][out] long *pValue)
        void Placeholder_GetLanguageID();
        //[propput]SetLanguageID([in] long Value)
        void Placeholder_SetLanguageID();
        //[propget]GetName([retval][out] BSTR *pbstr)
        //[propput]SetName([in] BSTR bstr)
        string Name { get; set; }
        //[propget]GetOutline([retval][out] long *pValue)
        //[propput]SetOutline([in] long Value)
        TomBool Outline { get; set; }
        //[propget]GetPosition([retval][out] float *pValue)
        void Placeholder_GetPosition();
        //[propput]SetPosition([in] float Value)
        void Placeholder_SetPosition();
        //[propget]GetProtected([retval][out] long *pValue)
        //[propput]SetProtected([in] long Value)
        TomBool Protected { get; set; }
        //[propget]GetShadow([retval][out] long *pValue)
        //[propput]SetShadow([in] long Value)
        TomBool Shadow { get; set; }
        //[propget]GetSize([retval][out] float *pValue)
        //[propput]SetSize([in] float Value)
        float Size { get; set; }
        //[propget]GetSmallCaps([retval][out] long *pValue)
        //[propput]SetSmallCaps([in] long Value)
        TomBool SmallCaps { get; set; }
        //[propget]GetSpacing([retval][out] float *pValue)
        void Placeholder_GetSpacing();
        //[propput]SetSpacing([in] float Value)
        void Placeholder_SetSpacing();
        //[propget]GetStrikeThrough([retval][out] long *pValue)
        //[propput]SetStrikeThrough([in] long Value)
        TomBool StrikeThrough { get; set; }
        //[propget]GetSubscript([retval][out] long *pValue)
        //[propput]SetSubscript([in] long Value)
        TomBool Subscript { get; set; }
        //[propget]GetSuperscript([retval][out] long *pValue)
        //[propput]SetSuperscript([in] long Value)
        TomBool Superscript { get; set; }
        //[propget]GetUnderline([retval][out] long *pValue)
        //[propput]SetUnderline([in] long Value)
        TomUnderline Underline { get; set; }
        //[propget]GetWeight([retval][out] long *pValue)
        //[propput]SetWeight([in] long Value)
        int Weight { get; set; }
    }

    [ComImport, Guid("8CC497C4-A1DF-11CE-8098-00AA0047BE5D"), InterfaceType(ComInterfaceType.InterfaceIsDual)]
    internal interface ITextPara
    {
        //[propget]GetDuplicate([retval][out]ITextPara * *ppPara)
        void Placeholder_GetDuplicate();
        //[propput]SetDuplicate([in]ITextPara *pPara)
        void Placeholder_SetDuplicate();
        //CanChange([retval][out]long *pB)
        void Placeholder_CanChange();
        //IsEqual([in]ITextPara *pPara,[retval][out]long *pB)
        void Placeholder_IsEqual();
        //Reset([in]long Value)
        void Placeholder_Reset();
        //[propget]GetStyle([retval][out]long *pValue)
        void Placeholder_GetStyle();
        //[propput]SetStyle([in]long Value)
        void Placeholder_SetStyle();
        //[propget]GetAlignment([retval][out]long *pValue)
        //[propput]SetAlignment([in]long Value)
        TomAlignment Alignment { get; set; }
        //[propget]GetHyphenation([retval][out]long *pValue)
        void Placeholder_GetHyphenation();
        //[propput]SetHyphenation([in]long Value)
        void Placeholder_SetHyphenation();
        //[propget]GetFirstLineIndent([retval][out]float *pValue)
        float FirstLineIndent { get; }
        //[propget]GetKeepTogether([retval][out]long *pValue)
        void Placeholder_GetKeepTogether();
        //[propput]SetKeepTogether([in]long Value)
        void Placeholder_SetKeepTogether();
        //[propget]GetKeepWithNext([retval][out]long *pValue)
        void Placeholder_GetKeepWithNext();
        //[propput]SetKeepWithNext([in]long Value)
        void Placeholder_SetKeepWithNext();
        //[propget]GetLeftIndent([retval][out]float *pValue)
        float LeftIndent { get; }
        //[propget]GetLineSpacing([retval][out]float *pValue)
        void Placeholder_GetLineSpacing();
        //[propget]GetLineSpacingRule([retval][out]long *pValue)
        void Placeholder_GetLineSpacingRule();
        //[propget]GetListAlignment([retval][out]long *pValue)
        void Placeholder_GetListAlignment();
        //[propput]SetListAlignment([in]long Value)
        void Placeholder_SetListAlignment();
        //[propget]GetListLevelIndex([retval][out]long *pValue)
        void Placeholder_GetListLevelIndex();
        //[propput]SetListLevelIndex([in]long Value)
        void Placeholder_SetListLevelIndex();
        //[propget]GetListStart([retval][out]long *pValue)
        void Placeholder_GetListStart();
        //[propput]SetListStart([in]long Value)
        void Placeholder_SetListStart();
        //[propget]GetListTab([retval][out]float *pValue)
        void Placeholder_GetListTab();
        //[propput]SetListTab([in]float Value)
        void Placeholder_SetListTab();
        //[propget]GetListType([retval][out]long *pValue)
        //[propput]SetListType([in]long Value)
        TomListType ListType { get; set; }
        //[propget]GetNoLineNumber([retval][out]long *pValue)
        void Placeholder_GetNoLineNumber();
        //[propput]SetNoLineNumber([in]long Value)
        void Placeholder_SetNoLineNumber();
        //[propget]GetPageBreakBefore([retval][out]long *pValue)
        void Placeholder_GetPageBreakBefore();
        //[propput]SetPageBreakBefore([in]long Value)
        void Placeholder_SetPageBreakBefore();
        //[propget]GetRightIndent([retval][out]float *pValue)
        //[propput]SetRightIndent([in]float Value)
        float RightIndent { get; set; }
        //SetIndents([in]float StartIndent,[in]float LeftIndent,[in]float RightIndent)
        void Placeholder_SetIndents();
        //SetLineSpacing([in]long LineSpacingRule,[in]float LineSpacing)
        void Placeholder_SetLineSpacing();
        //[propget]GetSpaceAfter([retval][out]float *pValue)
        void Placeholder_GetSpaceAfter();
        //[propput]SetSpaceAfter([in]float Value)
        void Placeholder_SetSpaceAfter();
        //[propget]GetSpaceBefore([retval][out]float *pValue)
        void Placeholder_GetSpaceBefore();
        //[propput]SetSpaceBefore([in]float Value)
        void Placeholder_SetSpaceBefore();
        //[propget]GetWidowControl([retval][out]long *pValue)
        void Placeholder_GetWidowControl();
        //[propput]SetWidowControl([in]long Value)
        void Placeholder_SetWidowControl();
        //[propget]GetTabCount([retval][out]long *pCount)
        int TabCount { get; }
        //AddTab([in]float tbPos,[in]long tbAlign,[in]long tbLeader)
        void Placeholder_AddTab();
        //ClearAllTabs(void)
        void Placeholder_ClearAllTabs();
        //DeleteTab([in]float tbPos)
        void Placeholder_DeleteTab();
        //GetTab([in]long iTab,[out]float *ptbPos,[out]long *ptbAlign,[out]long *ptbLeader)
        void GetTab(int iTab, out float tbPos, out TomAlignment tbAlign, out TomLeader tbLeader);
    }

    internal enum TomAlignment : int
    {
        tomUndefined = -9999999,
        tomAlignLeft = 0,
        tomAlignCenter = 1,
        tomAlignRight = 2,
        tomAlignJustify = 3,
        tomAlignDecimal = 3,
        tomAlignBar = 4,
    }

    internal enum TomAnimation : int
    {
        tomUndefined = -9999999,
        tomNoAnimation = 0,
        tomLasVegasLights = 1,
        tomBlinkingBackground = 2,
        tomSparkleText = 3,
        tomMarchingBlackAnts = 4,
        tomMarchingRedAnts = 5,
        tomShimmer = 6,
    }

    internal enum TomBool : int
    {
        tomUndefined = -9999999,
        tomTrue = -1,
        tomFalse = 0,
    }

    // miscellaneous integer constants
    internal enum TomConst : int
    {
        tomUndefined = -9999999,
        tomAutocolor = -9999997,
    }

    internal enum TomExtend : int
    {
        tomMove = 0,
        tomExtend = 1,
    }

    [Flags]
    internal enum TomGetPoint : int
    {
        //TA_LEFT = 0
        //TA_TOP = 0
        TA_RIGHT = 2,
        TA_CENTER = 6,
        TA_BOTTOM = 8,
        TA_BASELINE = 24,
        tomStart = 32,
    }

    internal enum TomLeader : int
    {
        tomDots = 1,
        tomDashes = 2,
        tomLines = 3,
    }

    // See ITextPara::GetListType documentation
    [Flags]
    internal enum TomListType : int
    {
        tomUndefined = -9999999,

        tomListNone = 0,
        tomListBullet = 1,
        tomListNumberAsArabic = 2,
        tomListNumberAsLCLetter = 3,
        tomListNumberAsUCLetter = 4,
        tomListNumberAsLCRoman = 5,
        tomListNumberAsUCRoman = 6,
        tomListNumberAsSequence = 7,
        tomListTypeMask = 0xf,

        tomListParentheses = 0x10000,
        tomListPeriod = 0x20000,
        tomListPlain = 0x30000,
        tomListFormatMask = 0xf0000,
    }

    [Flags]
    internal enum TomMatch : int
    {
        tomMatchWord = 2,
        tomMatchCase = 4,
        tomMatchPattern = 8,
    }

    // ITextSelection Flags values
    [Flags]
    internal enum TomSelectionFlags : int
    {
        tomSelStartActive = 1,
        tomSelAtEOL = 2,
        tomSelOvertype = 4,
        tomSelActive = 8,
        tomSelReplace = 16,
    }

    internal enum TomStartEnd : int
    {
        tomStart = 32,
        tomEnd = 0,
    }

    internal enum TomStory : int
    {
        tomUnknownStory = 0,
        tomMainTextStory = 1,
        tomFootnotesStory = 2,
        tomEndnotesStory = 3,
        tomCommentsStory = 4,
        tomTextFrameStory = 5,
        tomEvenPagesHeaderStory = 6,
        tomPrimaryHeaderStory = 7,
        tomEvenPagesFooterStory = 8,
        tomPrimaryFooterStory = 9,
        tomFirstPageHeaderStory = 10,
        tomFirstPageFooterStory = 11,
    }

    internal enum TomUnderline : int
    {
        tomUndefined = -9999999,
        tomTrue = -1,
        tomNone = 0,
        tomSingle = 1,
        tomWords = 2,
        tomDouble = 3,
        tomDotted = 4,
        tomDash = 5,
        tomDashDot = 6,
        tomDashDotDot = 7,
        tomWave = 8,
        tomThick = 9,
        tomHair = 10,
    }

    internal enum TomUnit : int
    {
        tomCharacter = 1,
        tomWord = 2,
        tomSentence = 3,
        tomParagraph = 4,
        tomLine = 5,
        tomStory = 6,
        tomScreen = 7,
        tomSection = 8,
        tomColumn = 9,
        tomRow = 10,
        tomWindow = 11,
        tomCell = 12,
        tomCharFormat = 13,
        tomParaFormat = 14,
        tomTable = 15,
        tomObject = 16,
    }
}
