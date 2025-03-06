///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilBitcodeReader.cs                                                      //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using DotNetDxc;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MainNs
{
    enum StandardWidths
    {
        BlockIDWidth = 8,  // We use VBR-8 for block IDs.
        CodeLenWidth = 4,  // Codelen are VBR-4.
        BlockSizeWidth = 32  // BlockSize up to 2^32 32-bit words = 16GB per block.
    };

    // The standard abbrev namespace always has a way to exit a block, enter a
    // nested block, define abbrevs, and define an unabbreviated record.
    enum FixedAbbrevIDs
    {
        END_BLOCK = 0,  // Must be zero to guarantee termination for broken bitcode.
        ENTER_SUBBLOCK = 1,

        /// DEFINE_ABBREV - Defines an abbrev for the current block.  It consists
        /// of a vbr5 for # operand infos.  Each operand info is emitted with a
        /// single bit to indicate if it is a literal encoding.  If so, the value is
        /// emitted with a vbr8.  If not, the encoding is emitted as 3 bits followed
        /// by the info value as a vbr5 if needed.
        DEFINE_ABBREV = 2,

        // UNABBREV_RECORDs are emitted with a vbr6 for the record code, followed by
        // a vbr6 for the # operands, followed by vbr6's for each operand.
        UNABBREV_RECORD = 3,

        // This is not a code, this is a marker for the first abbrev assignment.
        FIRST_APPLICATION_ABBREV = 4
    };

    /// StandardBlockIDs - All bitcode files can optionally include a BLOCKINFO
    /// block, which contains metadata about other blocks in the file.
    enum StandardBlockIDs
    {
        /// BLOCKINFO_BLOCK is used to define metadata about blocks, for example,
        /// standard abbrevs that should be available to all blocks of a specified
        /// ID.
        BLOCKINFO_BLOCK_ID = 0,

        // Block IDs 1-7 are reserved for future expansion.
        FIRST_APPLICATION_BLOCKID = 8
    };

    /// BlockInfoCodes - The blockinfo block contains metadata about user-defined
    /// blocks.
    enum BlockInfoCodes
    {
        // DEFINE_ABBREV has magic semantics here, applying to the current SETBID'd
        // block, instead of the BlockInfo block.

        BLOCKINFO_CODE_SETBID = 1, // SETBID: [blockid#]
        BLOCKINFO_CODE_BLOCKNAME = 2, // BLOCKNAME: [name]
        BLOCKINFO_CODE_SETRECORDNAME = 3  // BLOCKINFO_CODE_SETRECORDNAME:
                                          //                             [id, name]
    };

    public static class ByteArrayExtensions
    {
        public static int size<T>(this List<T> list)
        {
            return list.Count();
        }
        public static bool empty<T>(this IEnumerable<T> list)
        {
            return list.Count() == 0;
        }
        public static void push_back<T>(this List<T> list, T element)
        {
            list.Add(element);
        }
        public static void resize<T>(this List<T> list, int size)
        {
            if (list.Count > size)
                list.RemoveRange(size, list.Count - size);
            else
                while (list.Count < size)
                    list.Add(default(T));
                
        }
        public static void clear<T>(this List<T> list)
        {
            list.Clear();
        }

        public static TreeNode RangeNodeASCII(this byte[] bytes, string text, ref int offset, int charLength, out string value)
        {
            System.Diagnostics.Debug.Assert(offset % 8 == 0, "else NYI");
            int byteOffset = offset / 8;
            char[] valueChars = new char[charLength];
            for (int i = 0; i < charLength; ++i)
            {
                valueChars[i] = (char)bytes[byteOffset + i];
            }
            value = new string(valueChars);
            TreeNode result = TreeNodeRange.For(text + ": '" + value + "'", offset, charLength * 8);
            offset += charLength * 8;
            return result;
        }
    }

    public class BitCodeAbbrevOp
    {
        public enum EncKind
        {
            Fixed = 1,  // A fixed width field, Val specifies number of bits.
            VBR = 2,  // A VBR field where Val specifies the width of each chunk.
            Array = 3,  // A sequence of fields, next field species elt encoding.
            Char6 = 4,  // A 6-bit fixed field which maps to [a-zA-Z0-9._].
            Blob = 5   // 32-bit aligned array of 8-bit characters.
        }

        public BitCodeAbbrevOp(UInt64 val)
        {
            this.Val = val; this.IsLiteral = true;
        }
        public BitCodeAbbrevOp(EncKind K) : this(K, 0)
        {
        }
        public BitCodeAbbrevOp(EncKind K, UInt64 val)
        {
            this.Val = val; this.IsLiteral = false; this.Enc = K;
        }
        public UInt64 Val;
        public bool IsLiteral;
        public EncKind Enc;
        public bool isEncoding() { return !IsLiteral; }
        public EncKind getEncoding() { return Enc; }
        public bool hasEncodingData() { return hasEncodingData(getEncoding()); }
        public static bool hasEncodingData(EncKind E)
        {
            switch (E)
            {
                case EncKind.Fixed:
                case EncKind.VBR:
                    return true;
                case EncKind.Array:
                case EncKind.Char6:
                case EncKind.Blob:
                    return false;
            }
            throw new Exception("Invalid encoding");
        }

        /// isChar6 - Return true if this character is legal in the Char6 encoding.
        public static bool isChar6(char C)
        {
            if (C >= 'a' && C <= 'z') return true;
            if (C >= 'A' && C <= 'Z') return true;
            if (C >= '0' && C <= '9') return true;
            if (C == '.' || C == '_') return true;
            return false;
        }
        public static uint EncodeChar6(char C)
        {
            if (C >= 'a' && C <= 'z') return (uint)C - 'a';
            if (C >= 'A' && C <= 'Z') return (uint)C - 'A' + 26;
            if (C >= '0' && C <= '9') return (uint)C - '0' + 26 + 26;
            if (C == '.') return 62;
            if (C == '_') return 63;
            throw new Exception("Not a value Char6 character!");
        }

        public static char DecodeChar6(uint V)
        {
            //assert((V & ~63) == 0 && "Not a Char6 encoded character!");
            if (V < 26) return (char)(V + 'a');
            if (V < 26 + 26) return (char)(V - 26 + 'A');
            if (V < 26 + 26 + 10) return (char)(V - 26 - 26 + '0');
            if (V == 62) return '.';
            if (V == 63) return '_';
            throw new Exception("Not a value Char6 character!");
        }
    }

    public class BitCodeAbbrev
    {
        public readonly List<BitCodeAbbrevOp> OperandList = new List<BitCodeAbbrevOp>();
        public int getNumOperandInfos() { return OperandList.Count; }
    }

    public class BlockScope
    {
        public BlockScope(uint prevCodeSize)
        {
            this.PrevCodeSize = prevCodeSize;
        }
        public readonly uint PrevCodeSize;
        public readonly List<BitCodeAbbrev> PrevAbbrevs = new List<BitCodeAbbrev>();
    }

    public class DxilBitstreamReader
    {
        private readonly byte[] bytes;
        private readonly int length;
        private readonly int startOffset;

        /// <summary>
        /// This contains information emitted to BLOCKINFO_BLOCK blocks. These
        /// describe abbreviations that all blocks of the specified ID inherit.
        /// </summary>
        public class BlockInfo
        {
            public uint BlockID;
            public List<BitCodeAbbrev> Abbrevs = new List<BitCodeAbbrev>();
            public string Name;
            public List<KeyValuePair<uint, string>> RecordNames = new List<KeyValuePair<uint, string>>();
        };

        private List<BlockInfo> BlockInfoRecords = new List<BlockInfo>();

        /// This is set to true if we don't care about the block/record name
        /// information in the BlockInfo block. Only llvm-bcanalyzer uses this.
        public bool IgnoreBlockInfoNames; // always set to false for bitstream visualization

        public DxilBitstreamReader(byte[] bytes, int startOffset, int length)
        {
            // startOffset is reported in bits, 
            this.bytes = bytes;
            this.startOffset = startOffset;
            this.length = length;
        }

        /// <summary>Gets the byte  offset in underlying buffer, for analysis.</summary>
        public uint GetStartOffset()
        {
            return (uint)this.startOffset;
        }

        public bool isValidAddress(uint offset)
        {
            return offset < length;
        }
        public uint readBytes(byte[] BArray, uint size, uint address)
        {
            uint BytesRead = size;
            uint BufferSize = (uint)this.length;
            if (address >= BufferSize)
                return 0;

            uint End = address + BytesRead;
            if (End > BufferSize)
                End = BufferSize;

            //assert(static_cast<int64_t>(End - Address) >= 0);
            BytesRead = End - address;
            Array.Copy(this.bytes, address + this.startOffset, BArray, 0, BytesRead);
            //memcpy(Buf, address + FirstChar, BytesRead);
            return BytesRead;
        }

        //===--------------------------------------------------------------------===//
        // Block Manipulation
        //===--------------------------------------------------------------------===//

        /// Return true if we've already read and processed the block info block for
        /// this Bitstream. We only process it for the first cursor that walks over
        /// it.
        public bool hasBlockInfoRecords() { return BlockInfoRecords.Count > 0; }

        /// If there is block info for the specified ID, return it, otherwise return
        /// null.
        public BlockInfo getBlockInfo(uint BlockID)
        {
            for (int i = 0; i < this.BlockInfoRecords.Count; i++)
                if (this.BlockInfoRecords[i].BlockID == BlockID)
                    return this.BlockInfoRecords[i];
            return null;
        }

        public BlockInfo getOrCreateBlockInfo(uint BlockID)
        {
            BlockInfo result = getBlockInfo(BlockID);
            if (result != null)
                return result;
            result = new BlockInfo();
            result.BlockID = BlockID;
            this.BlockInfoRecords.Add(result);
            return result;
        }

        /// Takes block info from the other bitstream reader.
        ///
        /// This is a "take" operation because BlockInfo records are non-trivial, and
        /// indeed rather expensive.
        void takeBlockInfo(DxilBitstreamReader Other)
        {
            this.BlockInfoRecords = Other.BlockInfoRecords;
        }
    }

    public struct BitstreamEntry
    {
        public enum EntryKind {
            Error,    // Malformed bitcode was found.
            EndBlock, // We've reached the end of the current block, (or the end of the
                      // file, which is treated like a series of EndBlock records.
            SubBlock, // This is the start of a new subblock of a specific ID.
            Record    // This is a record with a specific AbbrevID.
        }

        public EntryKind Kind;
        public uint ID;

        public static BitstreamEntry getError()
        {
            BitstreamEntry E = new BitstreamEntry() { Kind = EntryKind.Error }; return E;
        }
        public static BitstreamEntry getEndBlock()
        {
            BitstreamEntry E = new BitstreamEntry() { Kind = EntryKind.EndBlock }; return E;
        }
        public static BitstreamEntry getSubBlock(uint ID)
        {
            BitstreamEntry E = new BitstreamEntry() { Kind = EntryKind.SubBlock, ID = ID }; return E;
        }
        public static BitstreamEntry getRecord(uint AbbrevID)
        {
            BitstreamEntry E = new BitstreamEntry() { Kind = EntryKind.Record, ID = AbbrevID }; return E;
        }
    };

    public class DxilBitstreamCursor
    {
        // word_t == 32-bits
        DxilBitcodeReader log;
        DxilBitstreamReader BitStream;
        uint NextChar;

        // The size of the bicode. 0 if we don't know it yet.
        uint Size;

        /// This is the current data we have pulled from the stream but have not
        /// returned to the client. This is specifically and intentionally defined to
        /// follow the word size of the host machine for efficiency. We use word_t in
        /// places that are aware of this to make it perfectly explicit what is going
        /// on.
        uint CurWord;

        /// This is the number of bits in CurWord that are valid. This is always from
        /// [0...bits_of(size_t)-1] inclusive.
        uint BitsInCurWord;

        // This is the declared size of code values used for the current block, in
        // bits.
        uint CurCodeSize;

        /// Abbrevs installed at in this block.
        List<BitCodeAbbrev> CurAbbrevs = new List<BitCodeAbbrev>();

        public class Block
        {
            public uint PrevCodeSize;
            public List<BitCodeAbbrev> PrevAbbrevs = new List<BitCodeAbbrev>();
            public Block(uint PCS)
            {
                this.PrevCodeSize = PCS;
            }
        }

        /// This tracks the codesize of parent blocks.
        List<Block> BlockScope = new List<Block>();

        const uint MaxChunkSize = 4 * 8;

        public DxilBitstreamCursor(DxilBitstreamReader reader, DxilBitcodeReader log)
        {
            this.log = log;
            init(reader);
        }

        private void init(DxilBitstreamReader R)
        {
            freeState();

            BitStream = R;
            NextChar = 0;
            Size = 0;
            BitsInCurWord = 0;
            CurCodeSize = 2;
        }

        /// <summary>Gets the bit offset in underlying byte buffer, for analysis.</summary>
        public uint GetOffset()
        {
            return this.BitStream.GetStartOffset() * 8 + (uint)GetCurrentBitNo();
            //GetCurrentBitNo
            //return (32 - this.BitsInCurWord) + this.NextChar * 8 + this.BitStream.GetStartOffset() * 8;
        }

        void freeState()
        {
            CurAbbrevs.Clear();
            BlockScope.Clear();
        }

        bool canSkipToPos(uint pos)
        {
            // pos can be skipped to if it is a valid address or one byte past the end.
            return pos == 0 || BitStream.isValidAddress(pos - 1);
        }

        public bool AtEndOfStream()
        {
            if (BitsInCurWord != 0)
                return false;
            if (Size != 0)
                return Size == NextChar;
            fillCurWord();
            return BitsInCurWord == 0;
        }

        /// Return the number of bits used to encode an abbrev #.
        uint getAbbrevIDWidth() { return CurCodeSize; }

        const uint CHAR_BIT = 8;
        /// Return the bit # of the bit we are reading.
        public UInt64 GetCurrentBitNo()
        {
            return NextChar * CHAR_BIT - BitsInCurWord;
        }

        DxilBitstreamReader getBitStreamReader() { return BitStream; }

        /// Flags that modify the behavior of advance().
        [Flags]
        public enum advance_flags
        {
            None = 0,
            /// If this flag is used, the advance() method does not automatically pop
            /// the block scope when the end of a block is reached.
            AF_DontPopBlockAtEnd = 1,

            /// If this flag is used, abbrev entries are returned just like normal
            /// records.
            AF_DontAutoprocessAbbrevs = 2
        };

        public BitstreamEntry advance()
        {
            return advance(advance_flags.None);
        }

        public string AbbrevIDName(uint code)
        {
            string result = "#" + code;
            switch (code)
            {
                case END_BLOCK:
                    result += "=END_BLOCK";
                    break;
                case ENTER_SUBBLOCK:
                    result += "=ENTER_SUBBLOCK";
                    break;
                case DEFINE_ABBREV:
                    result += "=DEFINE_ABBREV";
                    break;
            }
            return result;
        }

        /// Advance the current bitstream, returning the next entry in the stream.
        public BitstreamEntry advance(advance_flags Flags)
        {
            for (;;)
            {
                uint codeStart = this.GetOffset();
                uint Code = ReadCode();
                log.NR_SE_Leaf("BlockCode: " + AbbrevIDName(Code), codeStart, this.GetOffset());

                if (Code == (uint)(FixedAbbrevIDs.END_BLOCK))
                {
                    // Pop the end of the block unless Flags tells us not to.
                    if (advance_flags.AF_DontPopBlockAtEnd != (Flags & advance_flags.AF_DontPopBlockAtEnd) && ReadBlockEnd())
                        return BitstreamEntry.getError();
                    return BitstreamEntry.getEndBlock();
                }

                if (Code == (uint)(FixedAbbrevIDs.ENTER_SUBBLOCK))
                    return BitstreamEntry.getSubBlock(ReadSubBlockID());

                if (Code == (uint)(FixedAbbrevIDs.DEFINE_ABBREV) &&
                    (advance_flags.AF_DontAutoprocessAbbrevs != (Flags & advance_flags.AF_DontAutoprocessAbbrevs)))
                {
                    // We read and accumulate abbrev's, the client can't do anything with
                    // them anyway.
                    ReadAbbrevRecord();
                    continue;
                }

                return BitstreamEntry.getRecord(Code);
            }
        }

        public BitstreamEntry advanceSkippingSubblocks()
        {
            return advanceSkippingSubblocks(advance_flags.None);
        }

        /// This is a convenience function for clients that don't expect any
        /// subblocks. This just skips over them automatically.
        public BitstreamEntry advanceSkippingSubblocks(advance_flags Flags)
        {
            for (;;)
            {
                // If we found a normal entry, return it.
                BitstreamEntry Entry = advance(Flags);
                if (Entry.Kind != BitstreamEntry.EntryKind.SubBlock)
                    return Entry;

                // If we found a sub-block, just skip over it and check the next entry.
                if (SkipBlock())
                    return BitstreamEntry.getError();
            }
        }

        const uint sizeof_word_t = 4;

        /// Reset the stream to the specified bit number.
        public void JumpToBit(UInt64 BitNo)
        {
            uint ByteNo = (uint)(BitNo / 8) & ~(sizeof_word_t - 1);
            uint WordBitNo = (uint)(BitNo & (sizeof_word_t * 8 - 1));
            //assert(canSkipToPos(ByteNo) && "Invalid location");

            // Move the cursor to the right word.
            NextChar = ByteNo;
            BitsInCurWord = 0;

            // Skip over any bits that are already consumed.
            if (WordBitNo != 0)
                Read(WordBitNo);
        }

        void fillCurWord()
        {
            if (Size != 0 && NextChar >= Size)
                throw new Exception("Unexpected end of file");

            // Read the next word from the stream.
            byte[] BArray = new byte[sizeof_word_t];

            uint BytesRead = BitStream.readBytes(BArray, sizeof_word_t, NextChar);

            // If we run out of data, stop at the end of the stream.
            if (BytesRead == 0)
            {
                Size = NextChar;
                return;
            }

            if (Size > 0) throw new NotImplementedException();
            //CurWord =
            //    support::endian::read<word_t, support::little, support::unaligned>(
            //        Array);
            CurWord = (uint)(BArray[0] | (BArray[1] << 8) | (BArray[2] << 16) | (BArray[3] << 24));
            NextChar += BytesRead;
            BitsInCurWord = BytesRead * 8;
        }

        public uint Read(uint NumBits)
        {
            const uint BitsInWord = MaxChunkSize;

            //assert(NumBits && NumBits <= BitsInWord &&
            //       "Cannot return zero or more than BitsInWord bits!");

            const uint Mask = 0x1f;

            // If the field is fully contained by CurWord, return it quickly.
            if (BitsInCurWord >= NumBits)
            {
                int bitsToShift = (int)(BitsInWord - NumBits);
                uint RQuick = CurWord & (~(uint)0 >> bitsToShift);

                // Use a mask to avoid undefined behavior.
                CurWord >>= (int)(NumBits & Mask);

                BitsInCurWord -= NumBits;
                return RQuick;
            }

            uint R = (BitsInCurWord != 0) ? CurWord : 0;
            uint BitsLeft = NumBits - BitsInCurWord;

            fillCurWord();

            // If we run out of data, stop at the end of the stream.
            if (BitsLeft > BitsInCurWord)
                return 0;

            uint R2 = CurWord & (~(uint)0 >> (int)(BitsInWord - BitsLeft));

            // Use a mask to avoid undefined behavior.
            CurWord >>= (int)(BitsLeft & Mask);

            BitsInCurWord -= BitsLeft;

            R |= R2 << (int)(NumBits - BitsLeft);

            return R;
        }

        public uint Read(uint NumBits, string text)
        {
            uint start = this.GetOffset();
            uint result = Read(NumBits);
            log.NR_SE_Leaf(text + ": " + result, start, this.GetOffset());
            return result;
        }

        public uint ReadVBR(uint NumBits)
        {
            return ReadVBR(NumBits, null);
        }

        public uint ReadVBR(uint NumBits, string text)
        {
            uint start = this.GetOffset();
            uint Piece = Read(NumBits);
            if ((Piece & (1U << (int)(NumBits - 1))) == 0)
            {
                log.NR_SE_Leaf(text + ": " + Piece, start, this.GetOffset());
                return Piece;
            }

            uint Result = 0;
            uint NextBit = 0;
            for (;;)
            {
                Result |= (Piece & ((1U << (int)(NumBits - 1)) - 1)) << (int)NextBit;

                if ((Piece & (1U << (int)(NumBits - 1))) == 0)
                {
                    log.NR_SE_Leaf(text + ": " + Result, start, this.GetOffset());
                    return Result;
                }

                NextBit += NumBits - 1;
                Piece = Read(NumBits);
            }
        }

        ulong ReadVBR64(uint NumBits)
        {
            return ReadVBR64(NumBits, null);
        }

        // Read a VBR that may have a value up to 64-bits in size. The chunk size of
        // the VBR must still be <= 32 bits though.
        ulong ReadVBR64(uint NumBits, string text)
        {
            uint startOff = this.GetOffset();
            uint Piece = Read(NumBits);
            if ((Piece & (1U << (int)(NumBits - 1))) == 0)
            {
                log.NR_SE_Leaf(text + ": " + Piece, startOff, this.GetOffset());
                return (ulong)(Piece);
            }

            ulong Result = 0;
            uint NextBit = 0;
            for (;;)
            {
                Result |= (ulong)(Piece & ((1U << (int)(NumBits - 1)) - 1)) << (int)NextBit;

                if ((Piece & (1U << (int)(NumBits - 1))) == 0)
                {
                    log.NR_SE_Leaf(text + ": " + Result, startOff, this.GetOffset());
                    return Result;
                }

                NextBit += NumBits - 1;
                Piece = Read(NumBits);
            }
        }


        private void SkipToFourByteBoundary()
        {
            // If word_t is 64-bits and if we've read less than 32 bits, just dump
            // the bits we have up to the next 32-bit boundary.
            // We fix word_t to be 32-bits though, so this is a no-op.
            //if (sizeof(word_t) > 4 &&
            //    BitsInCurWord >= 32)
            //{
            //    CurWord >>= BitsInCurWord - 32;
            //    BitsInCurWord = 32;
            //    return;
            //}
            if (BitsInCurWord == 0) return;
            uint start = this.GetOffset();
            BitsInCurWord = 0;
            log.NR_SE_Leaf("four-byte pad", start, this.GetOffset());
        }

        public uint ReadCode()
        {
            return Read(CurCodeSize);
        }


        // Block header:
        //    [ENTER_SUBBLOCK, blockid, newcodelen, <align4bytes>, blocklen]

        /// Having read the ENTER_SUBBLOCK code, read the BlockID for the block.
        public uint ReadSubBlockID()
        {
            return ReadVBR((uint)StandardWidths.BlockIDWidth, "SubBlockID");
        }

        /// Having read the ENTER_SUBBLOCK abbrevid and a BlockID, skip over the body
        /// of this block. If the block record is malformed, return true.
        public bool SkipBlock()
        {
            // Read and ignore the codelen value.  Since we are skipping this block, we
            // don't care what code widths are used inside of it.
            ReadVBR((uint)StandardWidths.CodeLenWidth, "CodeSize");
            SkipToFourByteBoundary();
            uint NumFourBytes = Read((uint)StandardWidths.BlockSizeWidth, "BlockSize(4-bytes)");

            // Check that the block wasn't partially defined, and that the offset isn't
            // bogus.
            uint SkipTo = (uint)(GetCurrentBitNo() + NumFourBytes * 4 * 8);
            if (AtEndOfStream() || !canSkipToPos(SkipTo / 8))
                return true;

            JumpToBit(SkipTo);
            return false;
        }

        public bool EnterSubBlock(uint BlockID)
        {
            uint dontCare;
            return EnterSubBlock(BlockID, out dontCare);
        }

        /// Having read the ENTER_SUBBLOCK abbrevid, enter the block, and return true
        /// if the block has an error.
        public bool EnterSubBlock(uint BlockID, out uint NumWordsP)
        {
            // Save the current block's state on BlockScope.
            BlockScope.Add(new Block(CurCodeSize));
            BlockScope.Last().PrevAbbrevs.AddRange(CurAbbrevs);

            // Add the abbrevs specific to this block to the CurAbbrevs list.
            DxilBitstreamReader.BlockInfo Info = BitStream.getBlockInfo(BlockID);
            if (Info != null) {
                CurAbbrevs.AddRange(Info.Abbrevs);
            }

            // Get the codesize of this block.
            CurCodeSize = ReadVBR((int)StandardWidths.CodeLenWidth, "CodeSize");
            // We can't read more than MaxChunkSize at a time
            NumWordsP = 0;
            if (CurCodeSize > MaxChunkSize)
                return true;

            SkipToFourByteBoundary();
            NumWordsP = Read((int)StandardWidths.BlockSizeWidth, "NumWords");

            // Validate that this block is sane.
            return CurCodeSize == 0 || AtEndOfStream();
        }

        public bool ReadBlockEnd()
        {
            if (BlockScope.Count == 0) return true;

            // Block tail:
            //    [END_BLOCK, <align4bytes>]
            SkipToFourByteBoundary();

            popBlockScope();
            return false;
        }

        void popBlockScope()
        {
            var last = BlockScope[BlockScope.Count - 1];
            CurCodeSize = last.PrevCodeSize;
            CurAbbrevs = last.PrevAbbrevs;
            BlockScope.RemoveAt(BlockScope.Count - 1);
        }

        //===--------------------------------------------------------------------===//
        // Record Processing
        //===--------------------------------------------------------------------===//

        /// Return the abbreviation for the specified AbbrevId.
        public BitCodeAbbrev getAbbrev(uint AbbrevID) {
            uint AbbrevNo = AbbrevID - (uint)FixedAbbrevIDs.FIRST_APPLICATION_ABBREV;
            if (AbbrevNo >= CurAbbrevs.Count)
                throw new Exception("Invalid abbrev number");
            return CurAbbrevs[(int)AbbrevNo];
        }

        /// Read the current record and discard it.
        public void skipRecord(uint AbbrevID)
        {
            throw new NotImplementedException();
        }

        public uint readRecord(uint AbbrevID, List<ulong> Vals)
        {
            string dontCare;
            return readRecord(AbbrevID, Vals, out dontCare);
        }

        private const uint END_BLOCK = (uint)FixedAbbrevIDs.END_BLOCK;
        private const uint ENTER_SUBBLOCK = (uint)FixedAbbrevIDs.ENTER_SUBBLOCK;
        private const uint DEFINE_ABBREV = (uint)FixedAbbrevIDs.DEFINE_ABBREV;
        private const uint UNABBREV_RECORD = (uint)FixedAbbrevIDs.UNABBREV_RECORD;

        /// BlockInfoCodes - The blockinfo block contains metadata about user-defined
        /// blocks.
        // enum BlockInfoCodes
        // DEFINE_ABBREV has magic semantics here, applying to the current SETBID'd
        // block, instead of the BlockInfo block.
        const uint BLOCKINFO_CODE_SETBID = 1; // SETBID: [blockid#]
        const uint BLOCKINFO_CODE_BLOCKNAME = 2; // BLOCKNAME: [name]
        const uint BLOCKINFO_CODE_SETRECORDNAME = 3;  // BLOCKINFO_CODE_SETRECORDNAME: [id, name]

        const uint BLOCKINFO_BLOCK_ID = (uint)StandardBlockIDs.BLOCKINFO_BLOCK_ID;


        ulong readAbbreviatedField(BitCodeAbbrevOp Op) {
            Debug.Assert(!Op.IsLiteral);
            switch (Op.Enc)
            {
                case BitCodeAbbrevOp.EncKind.Array:
                    throw new Exception();
                case BitCodeAbbrevOp.EncKind.Blob:
                    throw new Exception();
                case BitCodeAbbrevOp.EncKind.Fixed:
                    return Read((uint)Op.Val);
                case BitCodeAbbrevOp.EncKind.VBR:
                    return ReadVBR64((uint)Op.Val);
                case BitCodeAbbrevOp.EncKind.Char6:
                    return BitCodeAbbrevOp.DecodeChar6(Read(6));
                default:
                    throw new Exception();
            }
        }


        public uint readRecord(uint AbbrevID, List<ulong> Vals, out string Blob)
        {
            Blob = null;
            if (AbbrevID == UNABBREV_RECORD)
            {
                log.NR_Push("UNABBREV_RECORD");
                uint UnabbrevCode = ReadVBR(6, "Code");
                uint NumElts = ReadVBR(6, "NumElts");
                for (uint i = 0; i != NumElts; ++i)
                    Vals.Add(ReadVBR64(6, "Val #" + i));
                log.NR_Pop();
                return UnabbrevCode;
            }

            BitCodeAbbrev Abbv = getAbbrev(AbbrevID);

            // Read the record code first.
            //assert(Abbv->getNumOperandInfos() != 0 && "no record code in abbreviation?");
            BitCodeAbbrevOp CodeOp = Abbv.OperandList[0];
            uint Code;
            if (CodeOp.IsLiteral)
                Code = (uint)CodeOp.Val;
            else
            {
                if (CodeOp.Enc == BitCodeAbbrevOp.EncKind.Array ||
                    CodeOp.Enc == BitCodeAbbrevOp.EncKind.Blob)
                    throw new Exception("Abbreviation starts with an Array or a Blob");
                Code = (uint)readAbbreviatedField(CodeOp);
            }

            for (int i = 1, e = Abbv.getNumOperandInfos(); i != e; ++i)
            {
                BitCodeAbbrevOp Op = Abbv.OperandList[i];
                if (Op.IsLiteral)
                {
                    Vals.Add(Op.Val);
                    continue;
                }

                if (Op.getEncoding() != BitCodeAbbrevOp.EncKind.Array &&
                    Op.getEncoding() != BitCodeAbbrevOp.EncKind.Blob)
                {
                    Vals.Add(readAbbreviatedField(Op));
                    continue;
                }

                if (Op.getEncoding() == BitCodeAbbrevOp.EncKind.Array)
                {
                    // Array case.  Read the number of elements as a vbr6.
                    uint ArrNumElts = ReadVBR(6);

                    // Get the element encoding.
                    if (i + 2 != e)
                        throw new Exception("Array op not second to last");
                    BitCodeAbbrevOp EltEnc = Abbv.OperandList[++i];
                    if (!EltEnc.isEncoding())
                        throw new Exception(
                            "Array element type has to be an encoding of a type");
                    if (EltEnc.getEncoding() == BitCodeAbbrevOp.EncKind.Array ||
                        EltEnc.getEncoding() == BitCodeAbbrevOp.EncKind.Blob)
                        throw new Exception("Array element type can't be an Array or a Blob");

                    // Read all the elements.
                    for (; ArrNumElts != 0; --ArrNumElts)
                        Vals.Add(readAbbreviatedField(EltEnc));
                    continue;
                }

                //assert(Op.getEncoding() == BitCodeAbbrevOp::Blob);
                // Blob case.  Read the number of bytes as a vbr6.
                uint NumElts = ReadVBR(6);
                SkipToFourByteBoundary();  // 32-bit alignment

                // Figure out where the end of this blob will be including tail padding.
                uint CurBitPos = (uint)GetCurrentBitNo();
                uint NewEnd = CurBitPos + ((NumElts + 3) & (~(uint)3)) * 8;

                // If this would read off the end of the bitcode file, just set the
                // record to empty and return.
                if (!canSkipToPos(NewEnd / 8))
                {
                    for (uint ec = 0; ec < NumElts; ++ec)
                        Vals.Add(0);
                    //NextChar = BitStream->getBitcodeBytes().getExtent();
                    //break;
                    throw new NotImplementedException();
                }

                // Otherwise, inform the streamer that we need these bytes in memory.
                //const char* Ptr = (const char*) BitStream->getBitcodeBytes().getPointer(CurBitPos / 8, NumElts);

                // If we can return a reference to the data, do so to avoid copying it.
                Blob = null;
                //if (Blob)
                {
                    // *Blob = StringRef(Ptr, NumElts);
                }
                //else
                {
                    // Otherwise, unpack into Vals with zero extension.
                    //for (; 0 != NumElts; --NumElts)
                        //Vals.Add((uint char) * Ptr++);
                    throw new NotImplementedException();
                }
                // Skip over tail padding.
                //JumpToBit(NewEnd);
            }

            return Code;
        }

        //===--------------------------------------------------------------------===//
        // Abbrev Processing
        //===--------------------------------------------------------------------===//
        public void ReadAbbrevRecord()
        {
            log.NR_Push("AbbrevRecord");
            BitCodeAbbrev Abbv = new BitCodeAbbrev();
            uint NumOpInfo = ReadVBR(5, "NumOps");
            for (uint i = 0; i != NumOpInfo; ++i)
            {
                bool IsLiteral = Read(1) != 0;
                if (IsLiteral)
                {
                    Abbv.OperandList.Add(new BitCodeAbbrevOp(ReadVBR64(8, "Literal")));
                    continue;
                }

                BitCodeAbbrevOp.EncKind E = (BitCodeAbbrevOp.EncKind)Read(3, "EncodingKind");
                log.NR_Note(E.ToString());
                if (BitCodeAbbrevOp.hasEncodingData(E))
                {
                    ulong Data = ReadVBR64(5, "Data");

                    // As a special case, handle fixed(0) (i.e., a fixed field with zero bits)
                    // and vbr(0) as a literal zero.  This is decoded the same way, and avoids
                    // a slow path in Read() to have to handle reading zero bits.
                    if ((E == BitCodeAbbrevOp.EncKind.Fixed || E == BitCodeAbbrevOp.EncKind.VBR) &&
                        Data == 0)
                    {
                        Abbv.OperandList.Add(new BitCodeAbbrevOp((ulong)0));
                        continue;
                    }

                    if ((E == BitCodeAbbrevOp.EncKind.Fixed || E == BitCodeAbbrevOp.EncKind.VBR) &&
                        Data > MaxChunkSize)
                        throw new Exception(
                            "Fixed or VBR abbrev record with size > MaxChunkData");

                    Abbv.OperandList.Add(new BitCodeAbbrevOp(E, Data));
                }
                else
                    Abbv.OperandList.Add(new BitCodeAbbrevOp(E));
            }

            if (Abbv.OperandList.Count == 0)
                throw new Exception("Abbrev record with no operands");
            CurAbbrevs.Add(Abbv);
            log.NR_Pop();
        }

        public bool ReadBlockInfoBlock()
        {
            // If this is the second stream to get to the block info block, skip it.
            if (BitStream.hasBlockInfoRecords())
                return SkipBlock();

            if (EnterSubBlock(BLOCKINFO_BLOCK_ID)) return true;

            List<ulong> Record = new List<ulong>();
            DxilBitstreamReader.BlockInfo CurBlockInfo = null;

            // Read all the records for this module.
            for (;;)
            {
                BitstreamEntry Entry = advanceSkippingSubblocks(advance_flags.AF_DontAutoprocessAbbrevs);

                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.SubBlock: // Handled for us already.
                    case BitstreamEntry.EntryKind.Error:
                        return true;
                    case BitstreamEntry.EntryKind.EndBlock:
                        return false;
                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read abbrev records, associate them with CurBID.
                if (Entry.ID == DEFINE_ABBREV)
                {
                    if (CurBlockInfo == null) return true;
                    ReadAbbrevRecord();

                    // ReadAbbrevRecord installs the abbrev in CurAbbrevs.  Move it to the
                    // appropriate BlockInfo.
                    int lastIdx = CurAbbrevs.Count - 1;
                    CurBlockInfo.Abbrevs.Add(CurAbbrevs[lastIdx]);
                    CurAbbrevs.RemoveAt(lastIdx);
                    continue;
                }

                // Read a record.
                Record.Clear();
                switch (readRecord(Entry.ID, Record))
                {
                    default: break;  // Default behavior, ignore unknown content.
                    case BLOCKINFO_CODE_SETBID:
                        if (Record.Count < 1) return true;
                        CurBlockInfo = BitStream.getOrCreateBlockInfo((uint)Record[0]);
                        break;
                    case BLOCKINFO_CODE_BLOCKNAME:
                        {
                            if (CurBlockInfo == null) return true;
                            string Name = "";
                            for (int i = 0, e = Record.Count; i != e; ++i)
                                Name += (char)Record[i];
                            CurBlockInfo.Name = Name;
                            break;
                        }
                    case BLOCKINFO_CODE_SETRECORDNAME:
                        {
                            if (CurBlockInfo == null) return true;
                            string Name = "";
                            for (int i = 1, e = Record.Count; i != e; ++i)
                                Name += (char)Record[i];
                            CurBlockInfo.RecordNames.Add(new KeyValuePair<uint,string>((uint)Record[0], Name));
                            break;
                        }
                }
            }
        }
    }


    public class DxilBitcodeReader
    {
        private byte[] bytes;
        private int offset;
        private TreeNode rootNode;
        private DxilBitstreamReader StreamFile;
        private DxilBitstreamCursor Stream;
        private ulong NextUnreadBit = 0;
        private bool SeenValueSymbolTable;
        private bool SeenFirstFunctionBody;
        private List<DxilFunction> FunctionsWithBodies = new List<DxilFunction>();
        private bool UseRelativeIDs;
        private Stack<TreeNode> nodes = new Stack<TreeNode>();

        public DxilBitcodeReader(byte[] bytes, int offset, int length, TreeNode root)
        {
            this.bytes = bytes;
            this.offset = offset;
            this.rootNode = root;
            this.nodes.Push(root);

            // Length and offset are in bits, but we initialize the bitstream
            // object in bytes.
            Debug.Assert(offset % 8 == 0);
            Debug.Assert(length % 8 == 0);
            this.StreamFile = new DxilBitstreamReader(bytes, offset / 8, length / 8);
            this.Stream = new DxilBitstreamCursor(this.StreamFile, this);
        }

        public static void BuildTree(byte[] bytes, ref int offset, int length, TreeNode root)
        {
            DxilBitcodeReader reader = new DxilBitcodeReader(bytes, offset, length, root);
            reader.parseBitcodeInto();
            offset = reader.offset;
        }

        public TreeNode NR_SE_Leaf(string text, uint start, uint end)
        {
            var tn = NR_SE(text, start, end);
            this.nodes.Peek().Nodes.Add(tn);
            return tn;
        }

        public TreeNode NR_Push(string text)
        {
            var tn = new TreeNode(text);
            this.nodes.Peek().Nodes.Add(tn);
            this.nodes.Push(tn);
            return tn;
        }

        public void NR_Note(string text)
        {
            this.nodes.Peek().Text += " - " + text;
        }

        public void NR_Pop()
        {
            this.nodes.Pop();
        }

        /// <summary>TreeNode with range for start/end.</summary>
        private TreeNode NR_SE(string text, uint start, uint end)
        {
            return TreeNodeRange.For(text, (int)start, (int)(end - start));
        }

        private void parseBitcodeInto()
        {
            // Sniff for the signature.
            uint sigStart = Stream.GetOffset();
            if (Stream.Read(8) != 'B' ||
                Stream.Read(8) != 'C' ||
                Stream.Read(4) != 0x0 ||
                Stream.Read(4) != 0xC ||
                Stream.Read(4) != 0xE ||
                Stream.Read(4) != 0xD)
                throw new Exception("Invalid bitcode signature");
            uint sigEnd = Stream.GetOffset();
            NR_SE_Leaf("Signature - BC 0xC0DE", sigStart, sigEnd);

            // We expect a number of well-defined blocks, though we don't necessarily
            // need to understand them all.
            for (;;)
            {
                if (Stream.AtEndOfStream())
                {
                    // We didn't really read a proper Module.
                    throw new Exception("Malformed IR file");
                }

                BitstreamEntry Entry =
                  Stream.advance(DxilBitstreamCursor.advance_flags.AF_DontAutoprocessAbbrevs);

                if (Entry.Kind != BitstreamEntry.EntryKind.SubBlock)
                    throw new Exception("Malformed block");

                if (Entry.ID == MODULE_BLOCK_ID)
                {
                    NR_Push("Module Block");
                    parseModule(false);
                    NR_Pop();
                    return;
                }

                if (Stream.SkipBlock())
                    throw new Exception("Invalid record");
            }
        }
        const uint BLOCKINFO_BLOCK_ID = (uint)StandardBlockIDs.BLOCKINFO_BLOCK_ID;

        // enum BlockIDs
        const uint MODULE_BLOCK_ID = 8;
        const uint PARAMATTR_BLOCK_ID = 9;
        const uint PARAMATTR_GROUP_BLOCK_ID = 10;
        const uint CONSTANTS_BLOCK_ID = 11;
        const uint FUNCTION_BLOCK_ID = 12;
        const uint UNUSED_ID1 = 13;
        const uint VALUE_SYMTAB_BLOCK_ID = 14;
        const uint METADATA_BLOCK_ID = 15;
        const uint METADATA_ATTACHMENT_ID = 16;
        const uint TYPE_BLOCK_ID_NEW = 17;
        const uint USELIST_BLOCK_ID = 18;
        // enum ModuleCodes
        const uint MODULE_CODE_VERSION = 1;    // VERSION:     [version#]
        const uint MODULE_CODE_TRIPLE = 2;    // TRIPLE:      [strchr x N]
        const uint MODULE_CODE_DATALAYOUT = 3;    // DATALAYOUT:  [strchr x N]
        const uint MODULE_CODE_ASM = 4;    // ASM:         [strchr x N]
        const uint MODULE_CODE_SECTIONNAME = 5;    // SECTIONNAME: [strchr x N]
        // FIXME: Remove DEPLIB in 4.0.
        const uint MODULE_CODE_DEPLIB = 6;    // DEPLIB:      [strchr x N]
        // GLOBALVAR: [pointer type, isconst, initid,
        //             linkage, alignment, section, visibility, threadlocal]
        const uint MODULE_CODE_GLOBALVAR = 7;
        // FUNCTION:  [type, callingconv, isproto, linkage, paramattrs, alignment,
        //             section, visibility, gc, unnamed_addr]
        const uint MODULE_CODE_FUNCTION = 8;
        // ALIAS: [alias type, aliasee val#, linkage, visibility]
        const uint MODULE_CODE_ALIAS = 9;
        // MODULE_CODE_PURGEVALS: [numvals]
        const uint MODULE_CODE_PURGEVALS = 10;
        const uint MODULE_CODE_GCNAME = 11;  // GCNAME: [strchr x N]
        const uint MODULE_CODE_COMDAT = 12;  // COMDAT: [selection_kind, name]

        /// PARAMATTR blocks have code for defining a parameter attribute set.
        // enum AttributeCodes
        // FIXME: Remove `PARAMATTR_CODE_ENTRY_OLD' in 4.0
        const uint PARAMATTR_CODE_ENTRY_OLD = 1; // ENTRY: [paramidx0, attr0, paramidx1, attr1...]
        const uint PARAMATTR_CODE_ENTRY = 2; // ENTRY: [paramidx0, attrgrp0, paramidx1, attrgrp1, ...]
        const uint PARAMATTR_GRP_CODE_ENTRY = 3; // ENTRY: [id, attr0, att1, ...]

        const bool ShouldLazyLoadMetadata = false;
        private void globalCleanup()
        {
            //throw new NotImplementedException();
        }
        private static bool convertToString(List<ulong> Record, int Idx, out string Result)
        {
            if (Idx > Record.Count)
            {
                Result = null; return true;
            }
            StringBuilder sb = new StringBuilder(Record.Count - Idx);
            for (int i = Idx; i < Record.Count; ++i) sb.Append((char)Record[i]);
            Result = sb.ToString();
            return false;
        }

        private Exception error(string text) { return new Exception(text); }
        private void parseModule(bool Resume)
        {
            if (Resume)
                Stream.JumpToBit(NextUnreadBit);
            else if (Stream.EnterSubBlock(MODULE_BLOCK_ID))
                throw new Exception("Invalid record");

            List<ulong> Record = new List<ulong>();
            List<string> SectionTable = new List<string>();
            List<string> GCTable = new List<string>();

            // Read all the records for this module.
            for (;;)
            {
                BitstreamEntry Entry = Stream.advance();

                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.Error:
                        throw new Exception("Malformed block");
                    case BitstreamEntry.EntryKind.EndBlock:
                        globalCleanup();
                        return;

                    case BitstreamEntry.EntryKind.SubBlock:
                        switch (Entry.ID)
                        {
                            default:  // Skip unknown content.
                                if (Stream.SkipBlock())
                                    throw error("Invalid record");
                                break;
                            case BLOCKINFO_BLOCK_ID:
                                NR_Push("Block Info");
                                if (Stream.ReadBlockInfoBlock())
                                    throw error("Malformed block");
                                NR_Pop();
                                break;
                            case PARAMATTR_BLOCK_ID:
                                NR_Push("Attribute");
                                parseAttributeBlock();
                                NR_Pop();
                                break;
                            case PARAMATTR_GROUP_BLOCK_ID:
                                NR_Push("AttributeGroup");
                                parseAttributeGroupBlock();
                                NR_Pop();
                                break;
                            case TYPE_BLOCK_ID_NEW:
                                NR_Push("TypeTable");
                                parseTypeTable();
                                NR_Pop();
                                break;
                            case VALUE_SYMTAB_BLOCK_ID:
                                parseValueSymbolTable();
                                SeenValueSymbolTable = true;
                                break;
                            case CONSTANTS_BLOCK_ID:
                                parseConstants();
                                resolveGlobalAndAliasInits();
                                break;
                            case METADATA_BLOCK_ID:
                                parseMetadata();
                                break;
                            case FUNCTION_BLOCK_ID:
                                // If this is the first function body we've seen, reverse the
                                // FunctionsWithBodies list.
                                if (!SeenFirstFunctionBody)
                                {
                                    FunctionsWithBodies.Reverse();
                                    globalCleanup();
                                    SeenFirstFunctionBody = true;
                                }

                                rememberAndSkipFunctionBody();
                                // Suspend parsing when we reach the function bodies. Subsequent
                                // materialization calls will resume it when necessary. If the bitcode
                                // file is old, the symbol table will be at the end instead and will not
                                // have been seen yet. In this case, just finish the parse now.
                                if (SeenValueSymbolTable)
                                {
                                    NextUnreadBit = Stream.GetCurrentBitNo();
                                    return;
                                }
                                break;
                            case USELIST_BLOCK_ID:
                                parseUseLists();
                                break;
                        }
                        continue;

                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read a record.
                switch (Stream.readRecord(Entry.ID, Record))
                {
                    default: break;  // Default behavior, ignore unknown content.
                    case MODULE_CODE_VERSION:
                        // VERSION: [version#]
                        if (Record.Count < 1)
                            throw error("Invalid record");
                        // Only version #0 and #1 are supported so far.
                        uint module_version = (uint)Record[0];
                        switch (module_version)
                        {
                            default:
                                throw error("Invalid value");
                            case 0:
                                UseRelativeIDs = false;
                                break;
                            case 1:
                                UseRelativeIDs = true;
                                break;
                        }
                        if (!UseRelativeIDs)
                        {
                            NR_Note("no relative ID support - old IR version");
                        }
                        break;
                    case MODULE_CODE_TRIPLE:
                        {  // TRIPLE: [strchr x N]
                            string S;
                            if (convertToString(Record, 0, out S))
                                throw error("Invalid record");
                            //TheModule->setTargetTriple(S);
                            break;
                        }
                    case MODULE_CODE_DATALAYOUT:
                        {  // DATALAYOUT: [strchr x N]
                            string S;
                            if (convertToString(Record, 0, out S))
                                throw error("Invalid record");
                            //TheModule->setDataLayout(S);
                            break;
                        }
                    case MODULE_CODE_ASM:
                        {  // ASM: [strchr x N]
                            string S;
                            if (convertToString(Record, 0, out S))
                                throw error("Invalid record");
                            //TheModule->setModuleInlineAsm(S);
                            break;
                        }
                    case MODULE_CODE_DEPLIB:
                        {  // DEPLIB: [strchr x N]
                           // FIXME: Remove in 4.0.
                            string S;
                            if (convertToString(Record, 0, out S))
                                throw error("Invalid record");
                            // Ignore value.
                            break;
                        }
                    case MODULE_CODE_SECTIONNAME:
                        {  // SECTIONNAME: [strchr x N]
                            string S;
                            if (convertToString(Record, 0, out S))
                                throw error("Invalid record");
                            SectionTable.Add(S);
                            break;
                        }
                    case MODULE_CODE_GCNAME:
                        {  // SECTIONNAME: [strchr x N]
                            string S;
                            if (convertToString(Record, 0, out S))
                                throw error("Invalid record");
                            GCTable.Add(S);
                            break;
                        }
                    case MODULE_CODE_COMDAT:
                        { // COMDAT: [selection_kind, name]
                            if (Record.Count < 2)
                                throw error("Invalid record");
                            throw new NotImplementedException();
#if FALSE
                            Comdat::SelectionKind SK = getDecodedComdatSelectionKind(Record[0]);
                            uint ComdatNameSize = Record[1];
                            std::string ComdatName;
                            ComdatName.reserve(ComdatNameSize);
                            for (uint i = 0; i != ComdatNameSize; ++i)
                                ComdatName += (char)Record[2 + i];
                            Comdat* C = TheModule->getOrInsertComdat(ComdatName);
                            C->setSelectionKind(SK);
                            ComdatList.push_back(C);
                            break;
#endif
                        }
                    // GLOBALVAR: [pointer type, isconst, initid,
                    //             linkage, alignment, section, visibility, threadlocal,
                    //             unnamed_addr, externally_initialized, dllstorageclass,
                    //             comdat]
                    case MODULE_CODE_GLOBALVAR:
                        {
                            if (Record.Count < 6)
                                throw error("Invalid record");
                            throw new NotImplementedException();
#if FALSE

                            Type* Ty = getTypeByID(Record[0]);
                            if (!Ty)
                                throw error("Invalid record");
                            bool isConstant = Record[1] & 1;
                            bool explicitType = Record[1] & 2;
                            uint AddressSpace;
                            if (explicitType)
                            {
                                AddressSpace = Record[1] >> 2;
                            }
                            else
                            {
                                if (!Ty->isPointerTy())
                                    throw error("Invalid type for value");
                                AddressSpace = cast<PointerType>(Ty)->getAddressSpace();
                                Ty = cast<PointerType>(Ty)->getElementType();
                            }

                            ulong RawLinkage = Record[3];
                            GlobalValue::LinkageTypes Linkage = getDecodedLinkage(RawLinkage);
                            uint Alignment;
                            parseAlignmentValue(Record[4], Alignment);
                            std::string Section;
                            if (Record[5])
                            {
                                if (Record[5] - 1 >= SectionTable.Count)
                                    throw error("Invalid ID");
                                Section = SectionTable[Record[5] - 1];
                            }
                            GlobalValue::VisibilityTypes Visibility = GlobalValue::DefaultVisibility;
                            // Local linkage must have default visibility.
                            if (Record.Count > 6 && !GlobalValue::isLocalLinkage(Linkage))
                                // FIXME: Change to an error if non-default in 4.0.
                                Visibility = getDecodedVisibility(Record[6]);

                            GlobalVariable::ThreadLocalMode TLM = GlobalVariable::NotThreadLocal;
                            if (Record.Count > 7)
                                TLM = getDecodedThreadLocalMode(Record[7]);

                            bool UnnamedAddr = false;
                            if (Record.Count > 8)
                                UnnamedAddr = Record[8];

                            bool ExternallyInitialized = false;
                            if (Record.Count > 9)
                                ExternallyInitialized = Record[9];

                            GlobalVariable* NewGV =
                              new GlobalVariable(*TheModule, Ty, isConstant, Linkage, nullptr, "", nullptr,
                                                 TLM, AddressSpace, ExternallyInitialized);
                            NewGV->setAlignment(Alignment);
                            if (!Section.empty())
                                NewGV->setSection(Section);
                            NewGV->setVisibility(Visibility);
                            NewGV->setUnnamedAddr(UnnamedAddr);

                            if (Record.Count > 10)
                                NewGV->setDLLStorageClass(getDecodedDLLStorageClass(Record[10]));
                            else
                                upgradeDLLImportExportLinkage(NewGV, RawLinkage);

                            ValueList.push_back(NewGV);

                            // Remember which value to use for the global initializer.
                            if (uint InitID = Record[2])
        GlobalInits.push_back(std::make_pair(NewGV, InitID - 1));

                            if (Record.Count > 11)
                            {
                                if (uint ComdatID = Record[11]) {
                                    if (ComdatID > ComdatList.Count)
                                        throw error("Invalid global variable comdat ID");
                                    NewGV->setComdat(ComdatList[ComdatID - 1]);
                                }
                            }
                            else if (hasImplicitComdat(RawLinkage))
                            {
                                NewGV->setComdat(reinterpret_cast<Comdat*>(1));
                            }
break;
#endif
                        }
                    // FUNCTION:  [type, callingconv, isproto, linkage, paramattr,
                    //             alignment, section, visibility, gc, unnamed_addr,
                    //             prologuedata, dllstorageclass, comdat, prefixdata]
                    case MODULE_CODE_FUNCTION:
                        {
                            if (Record.Count < 8)
                                throw error("Invalid record");

                            DxilType Ty = getTypeByID(Record[0]);
                            if (Ty == null)
                                throw error("Invalid record");
                            DxilPointerType PTy = Ty as DxilPointerType;
                            if (PTy != null)
                                Ty = PTy.getElementType();
                            DxilFunctionType FTy = Ty as DxilFunctionType;
                            if (FTy == null)
                                throw error("Invalid type for value");

                            //DxilFunction Func = Function::Create(FTy, GlobalValue::ExternalLinkage, "", TheModule);
                            DxilFunction Func = new DxilFunction(FTy);

#if FALSE
                            Func->setCallingConv(static_cast<CallingConv::ID>(Record[1]));
                            bool isProto = Record[2];
                            ulong RawLinkage = Record[3];
                            Func->setLinkage(getDecodedLinkage(RawLinkage));
                            Func->setAttributes(getAttributes(Record[4]));

                            uint Alignment;
                            parseAlignmentValue(Record[5], Alignment);
                            Func->setAlignment(Alignment);
                            if (Record[6])
                            {
                                if (Record[6] - 1 >= SectionTable.Count)
                                    throw error("Invalid ID");
                                Func->setSection(SectionTable[Record[6] - 1]);
                            }
                            // Local linkage must have default visibility.
                            if (!Func->hasLocalLinkage())
                                // FIXME: Change to an error if non-default in 4.0.
                                Func->setVisibility(getDecodedVisibility(Record[7]));
                            if (Record.Count > 8 && Record[8])
                            {
                                if (Record[8] - 1 >= GCTable.Count)
                                    throw error("Invalid ID");
                                Func->setGC(GCTable[Record[8] - 1].c_str());
                            }
                            bool UnnamedAddr = false;
                            if (Record.Count > 9)
                                UnnamedAddr = Record[9];
                            Func->setUnnamedAddr(UnnamedAddr);
                            if (Record.Count > 10 && Record[10] != 0)
                                FunctionPrologues.push_back(std::make_pair(Func, Record[10] - 1));

                            if (Record.Count > 11)
                                Func->setDLLStorageClass(getDecodedDLLStorageClass(Record[11]));
                            else
                                upgradeDLLImportExportLinkage(Func, RawLinkage);

                            if (Record.Count > 12)
                            {
                                if (uint ComdatID = Record[12]) {
                                    if (ComdatID > ComdatList.Count)
                                        throw error("Invalid function comdat ID");
                                    Func->setComdat(ComdatList[ComdatID - 1]);
                                }
                            }
                            else if (hasImplicitComdat(RawLinkage))
                            {
                                Func->setComdat(reinterpret_cast<Comdat*>(1));
                            }

                            if (Record.Count > 13 && Record[13] != 0)
                                FunctionPrefixes.push_back(std::make_pair(Func, Record[13] - 1));

                            if (Record.Count > 14 && Record[14] != 0)
                                FunctionPersonalityFns.push_back(std::make_pair(Func, Record[14] - 1));

                            ValueList.push_back(Func);

                            // If this is a function with a body, remember the prototype we are
                            // creating now, so that we can match up the body with them later.
                            if (!isProto)
                            {
                                Func->setIsMaterializable(true);
                                FunctionsWithBodies.push_back(Func);
                                DeferredFunctionInfo[Func] = 0;
                            }
#endif
                            break;
                        }
                    // ALIAS: [alias type, aliasee val#, linkage]
                    // ALIAS: [alias type, aliasee val#, linkage, visibility, dllstorageclass]
                    case MODULE_CODE_ALIAS:
                        {
                            if (Record.Count < 3)
                                throw error("Invalid record");
                            throw new NotImplementedException();
#if FALSE

                            Type* Ty = getTypeByID(Record[0]);
                            if (!Ty)
                                throw error("Invalid record");
                            auto* PTy = dyn_cast<PointerType>(Ty);
                            if (!PTy)
                                throw error("Invalid type for value");

                            auto* NewGA =
                                GlobalAlias::create(PTy, getDecodedLinkage(Record[2]), "", TheModule);
                            // Old bitcode files didn't have visibility field.
                            // Local linkage must have default visibility.
                            if (Record.Count > 3 && !NewGA->hasLocalLinkage())
                                // FIXME: Change to an error if non-default in 4.0.
                                NewGA->setVisibility(getDecodedVisibility(Record[3]));
                            if (Record.Count > 4)
                                NewGA->setDLLStorageClass(getDecodedDLLStorageClass(Record[4]));
                            else
                                upgradeDLLImportExportLinkage(NewGA, Record[2]);
                            if (Record.Count > 5)
                                NewGA->setThreadLocalMode(getDecodedThreadLocalMode(Record[5]));
                            if (Record.Count > 6)
                                NewGA->setUnnamedAddr(Record[6]);
                            ValueList.push_back(NewGA);
                            AliasInits.push_back(std::make_pair(NewGA, Record[1]));
                            break;
#endif
                        }
                    /// MODULE_CODE_PURGEVALS: [numvals]
                    case MODULE_CODE_PURGEVALS:
                        throw new NotImplementedException();
#if FALSE

                        // Trim down the value list to the specified size.
                        if (Record.Count < 1 || Record[0] > ValueList.Count)
                            throw error("Invalid record");
                        ValueList.shrinkTo(Record[0]);
#endif
                }
                Record.Clear();
            }
        }

        class AttributeSet
        {
        };

        class AttrBuilder
        {
        };


        private List<AttributeSet> MAttributes = new List<AttributeSet>();
        private Dictionary<uint, AttributeSet> MAttributeGroups = new Dictionary<uint, AttributeSet>();

        private void parseAttributeBlock()
        {
            if (Stream.EnterSubBlock(PARAMATTR_BLOCK_ID))
                throw error("Invalid record");

            if (!MAttributes.empty())
                throw error("Invalid multiple blocks");

            List<ulong> Record = new List<ulong>();

            List<AttributeSet> Attrs = new List<AttributeSet>();

            // Read all the records.
            for (;;)
            {
                BitstreamEntry Entry = Stream.advanceSkippingSubblocks();

                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.SubBlock: // Handled for us already.
                    case BitstreamEntry.EntryKind.Error:
                        throw error("Malformed block");
                    case BitstreamEntry.EntryKind.EndBlock:
                        return;
                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read a record.
                Record.Clear();
                switch (Stream.readRecord(Entry.ID, Record))
                {
                    default:  // Default behavior: ignore.
                        break;
                    case PARAMATTR_CODE_ENTRY_OLD:
                        { // ENTRY: [paramidx0, attr0, ...]
                          // FIXME: Remove in 4.0.
                            if ((Record.size() & 1) != 0)
                                throw error("Invalid record");

                            for (int i = 0, e = Record.size(); i != e; i += 2)
                            {
                                //AttrBuilder B;
                                //decodeLLVMAttributesForBitcode(B, Record[i + 1]);
                                //Attrs.push_back(AttributeSet::get(Context, Record[i], B));
                            }

                            //MAttributes.push_back(AttributeSet::get(Context, Attrs));
                            Attrs.clear();
                            break;
                        }
                    case PARAMATTR_CODE_ENTRY:
                        { // ENTRY: [attrgrp0, attrgrp1, ...]
                            for (int i = 0, e = Record.size(); i != e; ++i)
                                //Attrs.push_back(MAttributeGroups[Record[i]]);

                                //MAttributes.push_back(AttributeSet::get(Context, Attrs));
                                Attrs.clear();
                            break;
                        }
                }
            }
        }

        private void parseAttributeGroupBlock()
        {
            if (Stream.EnterSubBlock(PARAMATTR_GROUP_BLOCK_ID))
                throw error("Invalid record");

            if (!MAttributeGroups.empty())
                throw error("Invalid multiple blocks");

            List<ulong> Record = new List<ulong>();

            // Read all the records.
            for (;;)
            {
                BitstreamEntry Entry = Stream.advanceSkippingSubblocks();

                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.SubBlock: // Handled for us already.
                    case BitstreamEntry.EntryKind.Error:
                        throw error("Malformed block");
                    case BitstreamEntry.EntryKind.EndBlock:
                        return;
                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read a record.
                Record.Clear();
                switch (Stream.readRecord(Entry.ID, Record))
                {
                    default:  // Default behavior: ignore.
                        break;
                    case PARAMATTR_GRP_CODE_ENTRY:
                        { // ENTRY: [grpid, idx, a0, a1, ...]
                            if (Record.size() < 3)
                                throw error("Invalid record");

                            ulong GrpID = Record[0];
                            ulong Idx = Record[1]; // Index of the object this attribute refers to.

                            //AttrBuilder B;
                            for (int i = 2, e = Record.size(); i != e; ++i)
                            {
                                if (Record[i] == 0)
                                {        // Enum attribute
                                    ++i;
                                    //        AttrKind Kind;
                                    //        parseAttrKind(Record[++i], &Kind);
                                    //B.addAttribute(Kind);
                                }
                                else if (Record[i] == 1)
                                { // Integer attribute
                                    ++i;
                                    //Attribute::AttrKind Kind;
                                    //        parseAttrKind(Record[++i], &Kind);
                                    //if (Kind == Attribute::Alignment)
                                    //    B.addAlignmentAttr(Record[++i]);
                                    //else if (Kind == Attribute::StackAlignment)
                                    //    B.addStackAlignmentAttr(Record[++i]);
                                    //else if (Kind == Attribute::Dereferenceable)
                                    //    B.addDereferenceableAttr(Record[++i]);
                                    //else if (Kind == Attribute::DereferenceableOrNull)
                                    //    B.addDereferenceableOrNullAttr(Record[++i]);
                                }
                                else
                                {                     // String attribute
                                    Debug.Assert((Record[i] == 3 || Record[i] == 4),
                                           "Invalid attribute group entry");
                                    bool HasValue = (Record[i++] == 4);
                                    string KindStr = "";
                                    string ValStr = "";

                                    while (Record[i] != 0 && i != e)
                                        KindStr += Record[i++];
                                    Debug.Assert(Record[i] == 0, "Kind string not null terminated");

                                    if (HasValue)
                                    {
                                        // Has a value associated with it.
                                        ++i; // Skip the '0' that terminates the "kind" string.
                                        while (Record[i] != 0 && i != e)
                                            ValStr += Record[i++];
                                        Debug.Assert(Record[i] == 0, "Value string not null terminated");
                                    }

                                    //B.addAttribute(KindStr.str(), ValStr.str());
                                }
                            }

                            //MAttributeGroups[GrpID] = AttributeSet::get(Context, Idx, B);
                            break;
                        }
                }
            }
        }
        private void parseTypeTable()
        {
            if (Stream.EnterSubBlock(TYPE_BLOCK_ID_NEW))
                throw error("Invalid record");

            parseTypeTableBody();
        }

        class DxilType
        {
            public DxilType(string name) { this.Name = name; }
            public string Name;
            public static DxilType getInt8Ty()
            {
                return Int8Ty;
            }
            public static DxilType getInt32Ty()
            {
                return Int32Ty;
            }
            public static DxilType Int32Ty = new DxilType("i32");
            public static DxilType Int8Ty = new DxilType("i8");
        }
        class DxilPointerType : DxilType
        {
            private uint addressSpace;
            private DxilType elementType;
            public DxilPointerType(uint addressSpace, DxilType elementType) : base(elementType == null ? "?" : elementType.Name + "*")
            {
                this.addressSpace = addressSpace;
                this.elementType = elementType;
            }
            public DxilType getElementType() { return elementType; }
        }
        class DxilFunctionType : DxilType
        {
            public DxilFunctionType(DxilType ResultTy, DxilType[] ArgTys, bool varArgs) : base("Fn")
            {
                this.ResultTy = ResultTy;
                this.ArgTys = ArgTys;
                this.VarArgs = varArgs;
            }
            public DxilType ResultTy;
            public DxilType[] ArgTys;
            public bool VarArgs;
        }
        class DxilFunction
        {
            public DxilFunction(DxilFunctionType Ty)
            {
                this.ty = Ty;
            }

            private DxilFunctionType ty;
        }

        List<DxilType> TypeList = new List<DxilType>();
        List<object> ValueList = new List<object>();

        /// TYPE blocks have codes for each type primitive they use.
        // enum TypeCodes
        const uint TYPE_CODE_NUMENTRY = 1;    // NUMENTRY: [numentries]
                                              // Type Codes
        const uint TYPE_CODE_VOID = 2;    // VOID
        const uint TYPE_CODE_FLOAT = 3;    // FLOAT
        const uint TYPE_CODE_DOUBLE = 4;    // DOUBLE
        const uint TYPE_CODE_LABEL = 5;    // LABEL
        const uint TYPE_CODE_OPAQUE = 6;    // OPAQUE
        const uint TYPE_CODE_INTEGER = 7;    // INTEGER: [width]
        const uint TYPE_CODE_POINTER = 8;    // POINTER: [pointee type]

        const uint TYPE_CODE_FUNCTION_OLD = 9; // FUNCTION: [vararg, attrid, retty,
                                               //            paramty x N]

        const uint TYPE_CODE_HALF = 10;   // HALF

        const uint TYPE_CODE_ARRAY = 11;    // ARRAY: [numelts, eltty]
        const uint TYPE_CODE_VECTOR = 12;    // VECTOR: [numelts, eltty]

        // These are not with the other floating point types because they're
        // a late addition, and putting them in the right place breaks
        // binary compatibility.
        const uint TYPE_CODE_X86_FP80 = 13;    // X86 LONG DOUBLE
        const uint TYPE_CODE_FP128 = 14;    // LONG DOUBLE (112 bit mantissa)
        const uint TYPE_CODE_PPC_FP128 = 15;    // PPC LONG DOUBLE (2 doubles)

        const uint TYPE_CODE_METADATA = 16;    // METADATA

        const uint TYPE_CODE_X86_MMX = 17;     // X86 MMX

        const uint TYPE_CODE_STRUCT_ANON = 18; // STRUCT_ANON: [ispacked, eltty x N]
        const uint TYPE_CODE_STRUCT_NAME = 19; // STRUCT_NAME: [strchr x N]
        const uint TYPE_CODE_STRUCT_NAMED = 20;// STRUCT_NAMED: [ispacked, eltty x N]
        const uint TYPE_CODE_FUNCTION = 21; // FUNCTION: [vararg, retty, paramty x N]

        private DxilType getTypeByID(ulong id)
        {
            // Consider: if entry is null, should create placeholder struct
            return TypeList[(int)id];
        }

        private void parseTypeTableBody()
        {
            if (!TypeList.empty())
                throw error("Invalid multiple blocks");

            List<ulong> Record = new List<ulong>();
            int NumRecords = 0;

            string TypeName = "";

            // Read all the records for this type table.
            for (;;)
            {
                BitstreamEntry Entry = Stream.advanceSkippingSubblocks();

                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.SubBlock: // Handled for us already.
                    case BitstreamEntry.EntryKind.Error:
                        throw error("Malformed block");
                    case BitstreamEntry.EntryKind.EndBlock:
                        if (NumRecords != TypeList.size())
                            throw error("Malformed block");
                        return;
                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read a record.
                Record.clear();
                DxilType ResultTy = null;
                switch (Stream.readRecord(Entry.ID, Record))
                {
                    default:
                        throw error("Invalid value");
                    case TYPE_CODE_NUMENTRY: // TYPE_CODE_NUMENTRY: [numentries]
                                                   // TYPE_CODE_NUMENTRY contains a count of the number of types in the
                                                   // type list.  This allows us to reserve space.
                        if (Record.size() < 1)
                            throw error("Invalid record");
                        TypeList.resize((int)Record[0]);
                        continue;
                    case TYPE_CODE_VOID:      // VOID
                        ResultTy = new DxilType("void");
                        break;
                    case TYPE_CODE_HALF:     // HALF
                        ResultTy = new DxilType("HalfTy");
                        break;
                    case TYPE_CODE_FLOAT:     // FLOAT
                        ResultTy = new DxilType("Float");
                        break;
                    case TYPE_CODE_DOUBLE:    // DOUBLE
                        ResultTy = new DxilType("Double");
                        break;
                    case TYPE_CODE_X86_FP80:  // X86_FP80
                        ResultTy = new DxilType("X86_FP80");
                        break;
                    case TYPE_CODE_FP128:     // FP128
                        ResultTy = new DxilType("FP128");
                        break;
                    case TYPE_CODE_PPC_FP128: // PPC_FP128
                        ResultTy = new DxilType("PPC_FP128");
                        break;
                    case TYPE_CODE_LABEL:     // LABEL
                        ResultTy = new DxilType("Label");
                        break;
                    case TYPE_CODE_METADATA:  // METADATA
                        ResultTy = new DxilType("Metadata");
                        break;
                    case TYPE_CODE_X86_MMX:   // X86_MMX
                        ResultTy = new DxilType("X86_MMX");
                        break;
                    case TYPE_CODE_INTEGER:
                        { // INTEGER: [width]
                            if (Record.size() < 1)
                                throw error("Invalid record");

                            ulong NumBits = Record[0];
                            //if (NumBits < IntegerType::MIN_INT_BITS ||
                            //    NumBits > IntegerType::MAX_INT_BITS)
                            //    throw error("Bitwidth for integer type out of range");
                            ResultTy = new DxilType("int:" + NumBits);
                            break;
                        }
                    case TYPE_CODE_POINTER:
                        { // POINTER: [pointee type] or
                          //          [pointee type, address space]
                            if (Record.size() < 1)
                                throw error("Invalid record");
                            uint AddressSpace = 0;
                            if (Record.size() == 2)
                                AddressSpace = (uint)Record[1];
                            ResultTy = getTypeByID(Record[0]);
                            //if (!ResultTy || !PointerType::isValidElementType(ResultTy))
                            //    throw error("Invalid type");
                            ResultTy = new DxilPointerType(AddressSpace, ResultTy);
                            break;
                        }
                    case TYPE_CODE_FUNCTION_OLD:
                        {
                            // FIXME: attrid is dead, remove it in LLVM 4.0
                            // FUNCTION: [vararg, attrid, retty, paramty x N]
                            if (Record.size() < 3)
                                throw error("Invalid record");
                            throw new NotImplementedException();
#if FALSE
                            List<DxilType> ArgTys = new List<DxilType>();
                            for (int i = 3, e = Record.size(); i != e; ++i)
                            {
                                if (DxilType T = getTypeByID(Record[i]))
                                    ArgTys.push_back(T);
                                else
                                    break;
                            }

                            ResultTy = getTypeByID(Record[2]);
                            if (!ResultTy || ArgTys.size() < Record.size() - 3)
                                throw error("Invalid type");

                            ResultTy = FunctionType::get(ResultTy, ArgTys, Record[0]);
                            break;
#endif
                        }
                    case TYPE_CODE_FUNCTION:
                        {
                            // FUNCTION: [vararg, retty, paramty x N]
                            if (Record.size() < 2)
                                throw error("Invalid record");
                            List<DxilType> ArgTys = new List<DxilType>();
                            for (int i = 2, e = Record.size(); i != e; ++i)
                            {
                                //if (DxilType T =  getTypeByID(Record[i]))
                                DxilType T = getTypeByID(Record[i]);
                                if (T != null)
                                {
                                    //if (!FunctionType::isValidArgumentType(T))
                                    //    throw error("Invalid function argument type");
                                    ArgTys.push_back(T);
                                }
                                else
                                    break;
                            }

                            ResultTy = getTypeByID(Record[1]);
                            if (ResultTy == null || ArgTys.size() < Record.size() - 2)
                                throw error("Invalid type");

                            ResultTy = new DxilFunctionType(ResultTy, ArgTys.ToArray(), Record[0] != 0);
                            break;
                        }
                    case TYPE_CODE_STRUCT_ANON:
                        {  // STRUCT: [ispacked, eltty x N]
                            if (Record.size() < 1)
                                throw error("Invalid record");
                            List<DxilType> EltTys = new List<DxilType>();
                            for (int i = 1, e = Record.size(); i != e; ++i)
                            {
                                DxilType T = getTypeByID(Record[i]);
                                if (T != null)
                                    EltTys.push_back(T);
                                else
                                    break;
                            }
                            if (EltTys.size() != Record.size() - 1)
                                throw error("Invalid type");
                            ResultTy = new DxilType("StructType::get(Context, EltTys, Record[0]);");
                            break;
                        }
                    case TYPE_CODE_STRUCT_NAME:   // STRUCT_NAME: [strchr x N]
                        if (convertToString(Record, 0, out TypeName))
                            throw error("Invalid record");
                        continue;

                    case TYPE_CODE_STRUCT_NAMED:
                        { // STRUCT: [ispacked, eltty x N]
                            if (Record.size() < 1)
                                throw error("Invalid record");

                            if (NumRecords >= TypeList.size())
                                throw error("Invalid TYPE table");

                            // Check to see if this was forward referenced, if so fill in the temp.
                            //StructType* Res = cast_or_null<StructType>(TypeList[NumRecords]);
                            //if (Res)
                            //{
                            //    Res->setName(TypeName);
                            //    TypeList[NumRecords] = nullptr;
                            //}
                            //else  // Otherwise, create a new struct.
                            //    Res = createIdentifiedStructType(Context, TypeName);
                            //TypeName = "";

                            //SmallVector < Type *, 8 > EltTys;
                            //for (unsigned i = 1, e = Record.size(); i != e; ++i)
                            //{
                            //    if (Type * T = getTypeByID(Record[i]))
                            //        EltTys.push_back(T);
                            //    else
                            //        break;
                            //}
                            //if (EltTys.size() != Record.size() - 1)
                            //    throw error("Invalid record");
                            //Res->setBody(EltTys, Record[0]);
                            //ResultTy = Res;
                            break;
                        }
                    case TYPE_CODE_OPAQUE:
                        {       // OPAQUE: []
                            if (Record.size() != 1)
                                throw error("Invalid record");

                            if (NumRecords >= TypeList.size())
                                throw error("Invalid TYPE table");

                            // Check to see if this was forward referenced, if so fill in the temp.
                            //StructType* Res = cast_or_null<StructType>(TypeList[NumRecords]);
                            //if (Res)
                            //{
                            //    Res->setName(TypeName);
                            //    TypeList[NumRecords] = nullptr;
                            //}
                            //else  // Otherwise, create a new struct with no body.
                            //    Res = createIdentifiedStructType(Context, TypeName);
                            //TypeName = "";
                            //ResultTy = Res;
                            break;
                        }
                    case TYPE_CODE_ARRAY:     // ARRAY: [numelts, eltty]
                        if (Record.size() < 2)
                            throw error("Invalid record");
                        ResultTy = getTypeByID(Record[1]);
                        if (ResultTy == null) //|| !ArrayType::isValidElementType(ResultTy))
                            throw error("Invalid type");
                        ResultTy = new DxilType("ArrayType::get(ResultTy, Record[0]);");
                        break;
                    case TYPE_CODE_VECTOR:    // VECTOR: [numelts, eltty]
                        if (Record.size() < 2)
                            throw error("Invalid record");
                        if (Record[0] == 0)
                            throw error("Invalid vector length");
                        ResultTy = getTypeByID(Record[1]);
                        if (ResultTy == null) //|| !StructType::isValidElementType(ResultTy))
                            throw error("Invalid type");
                        ResultTy = new DxilType("VectorType::get(ResultTy, Record[0]);");
                        break;
                }

                if (NumRecords >= TypeList.size())
                    throw error("Invalid TYPE table");
                if (TypeList[NumRecords] != null)
                    throw error("Invalid TYPE table: Only named structs can be forward referenced");
                //Debug.Assert(ResultTy != null, "Didn't read a type?");
                TypeList[NumRecords++] = ResultTy;
            }
        }

        private void parseValueSymbolTable()
        {
            if (Stream.EnterSubBlock(VALUE_SYMTAB_BLOCK_ID))
                throw error("Invalid record");

            List<ulong> Record = new List<ulong>();

            //Triple TT(TheModule->getTargetTriple());

            // Read all the records for this value table.
            //SmallString < 128 > ValueName;
            for(;;)
            {
                BitstreamEntry Entry = Stream.advanceSkippingSubblocks(0);

                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.SubBlock: // Handled for us already.
                    case BitstreamEntry.EntryKind.Error:
                        throw error("Malformed block");
                    case BitstreamEntry.EntryKind.EndBlock:
                        return;
                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read a record.
                Record.clear();
#if FALSE
                switch (Stream.readRecord(Entry.ID, Record))
                {
                    default:  // Default behavior: unknown type.
                        break;
                    case bitc::VST_CODE_ENTRY:
                        {  // VST_ENTRY: [valueid, namechar x N]
                            if (convertToString(Record, 1, ValueName))
                                return error("Invalid record");
                            unsigned ValueID = Record[0];
                            if (ValueID >= ValueList.size() || !ValueList[ValueID])
                                return error("Invalid record");
                            Value* V = ValueList[ValueID];

                            V->setName(StringRef(ValueName.data(), ValueName.size()));
                            if (auto * GO = dyn_cast<GlobalObject>(V))
                            {
                                if (GO->getComdat() == reinterpret_cast<Comdat*>(1))
                                {
                                    if (TT.isOSBinFormatMachO())
                                        GO->setComdat(nullptr);
                                    else
                                        GO->setComdat(TheModule->getOrInsertComdat(V->getName()));
                                }
                            }
                            ValueName.clear();
                            break;
                        }
                    case bitc::VST_CODE_BBENTRY:
                        {
                            if (convertToString(Record, 1, ValueName))
                                return error("Invalid record");
                            BasicBlock* BB = getBasicBlock(Record[0]);
                            if (!BB)
                                return error("Invalid record");

                            BB->setName(StringRef(ValueName.data(), ValueName.size()));
                            ValueName.clear();
                            break;
                        }
                }
#else
                Stream.readRecord(Entry.ID, Record);
#endif
            }
        }
        private void parseConstants()
        {
            if (Stream.EnterSubBlock(CONSTANTS_BLOCK_ID))
                throw error("Invalid record");
            List<ulong> Record = new List<ulong>();

            // Read all the records for this value table.
            DxilType CurTy = DxilType.getInt32Ty();
            //uint NextCstNo = ValueList.size();
            for(;;)
            {
                BitstreamEntry Entry = Stream.advanceSkippingSubblocks();

                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.SubBlock: // Handled for us already.
                    case BitstreamEntry.EntryKind.Error:
                        throw error("Malformed block");
                    case BitstreamEntry.EntryKind.EndBlock:
                        //if (NextCstNo != ValueList.size())
                        //    throw error("Invalid ronstant reference");

                        // Once all the constants have been read, go through and resolve forward
                        // references.
                        //ValueList.resolveConstantForwardRefs();
                        return;
                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read a record.
                Record.clear();
#if FALSE
                Value* V = nullptr;
                unsigned BitCode = Stream.readRecord(Entry.ID, Record);
                switch (BitCode)
                {
                    default:  // Default behavior: unknown constant
                    case bitc::CST_CODE_UNDEF:     // UNDEF
                        V = UndefValue::get(CurTy);
                        break;
                    case bitc::CST_CODE_SETTYPE:   // SETTYPE: [typeid]
                        if (Record.empty())
                            return error("Invalid record");
                        if (Record[0] >= TypeList.size() || !TypeList[Record[0]])
                            return error("Invalid record");
                        CurTy = TypeList[Record[0]];
                        continue;  // Skip the ValueList manipulation.
                    case bitc::CST_CODE_NULL:      // NULL
                        V = Constant::getNullValue(CurTy);
                        break;
                    case bitc::CST_CODE_INTEGER:   // INTEGER: [intval]
                        if (!CurTy->isIntegerTy() || Record.empty())
                            return error("Invalid record");
                        V = ConstantInt::get(CurTy, decodeSignRotatedValue(Record[0]));
                        break;
                    case bitc::CST_CODE_WIDE_INTEGER:
                        {// WIDE_INTEGER: [n x intval]
                            if (!CurTy->isIntegerTy() || Record.empty())
                                return error("Invalid record");

                            APInt VInt =
                                readWideAPInt(Record, cast<IntegerType>(CurTy)->getBitWidth());
                            V = ConstantInt::get(Context, VInt);

                            break;
                        }
                    case bitc::CST_CODE_FLOAT:
                        {    // FLOAT: [fpval]
                            if (Record.empty())
                                return error("Invalid record");
                            if (CurTy->isHalfTy())
                                V = ConstantFP::get(Context, APFloat(APFloat::IEEEhalf,
                                                                     APInt(16, (uint16_t)Record[0])));
                            else if (CurTy->isFloatTy())
                                V = ConstantFP::get(Context, APFloat(APFloat::IEEEsingle,
                                                                     APInt(32, (uint32_t)Record[0])));
                            else if (CurTy->isDoubleTy())
                                V = ConstantFP::get(Context, APFloat(APFloat::IEEEdouble,
                                                                     APInt(64, Record[0])));
                            else if (CurTy->isX86_FP80Ty())
                            {
                                // Bits are not stored the same way as a normal i80 APInt, compensate.
                                uint64_t Rearrange[2];
                                Rearrange[0] = (Record[1] & 0xffffLL) | (Record[0] << 16);
                                Rearrange[1] = Record[0] >> 48;
                                V = ConstantFP::get(Context, APFloat(APFloat::x87DoubleExtended,
                                                                     APInt(80, Rearrange)));
                            }
                            else if (CurTy->isFP128Ty())
                                V = ConstantFP::get(Context, APFloat(APFloat::IEEEquad,
                                                                     APInt(128, Record)));
                            else if (CurTy->isPPC_FP128Ty())
                                V = ConstantFP::get(Context, APFloat(APFloat::PPCDoubleDouble,
                                                                     APInt(128, Record)));
                            else
                                V = UndefValue::get(CurTy);
                            break;
                        }

                    case bitc::CST_CODE_AGGREGATE:
                        {// AGGREGATE: [n x value number]
                            if (Record.empty())
                                return error("Invalid record");

                            unsigned Size = Record.size();
                            SmallVector < Constant *, 16 > Elts;

                            if (StructType * STy = dyn_cast<StructType>(CurTy))
                            {
                                for (unsigned i = 0; i != Size; ++i)
                                    Elts.push_back(ValueList.getConstantFwdRef(Record[i],
                                                                               STy->getElementType(i)));
                                V = ConstantStruct::get(STy, Elts);
                            }
                            else if (ArrayType * ATy = dyn_cast<ArrayType>(CurTy))
                            {
                                Type* EltTy = ATy->getElementType();
                                for (unsigned i = 0; i != Size; ++i)
                                    Elts.push_back(ValueList.getConstantFwdRef(Record[i], EltTy));
                                V = ConstantArray::get(ATy, Elts);
                            }
                            else if (VectorType * VTy = dyn_cast<VectorType>(CurTy))
                            {
                                Type* EltTy = VTy->getElementType();
                                for (unsigned i = 0; i != Size; ++i)
                                    Elts.push_back(ValueList.getConstantFwdRef(Record[i], EltTy));
                                V = ConstantVector::get(Elts);
                            }
                            else
                            {
                                V = UndefValue::get(CurTy);
                            }
                            break;
                        }
                    case bitc::CST_CODE_STRING:    // STRING: [values]
                    case bitc::CST_CODE_CSTRING:
                        { // CSTRING: [values]
                            if (Record.empty())
                                return error("Invalid record");

                            SmallString < 16 > Elts(Record.begin(), Record.end());
                            V = ConstantDataArray::getString(Context, Elts,
                                                             BitCode == bitc::CST_CODE_CSTRING);
                            break;
                        }
                    case bitc::CST_CODE_DATA:
                        {// DATA: [n x value]
                            if (Record.empty())
                                return error("Invalid record");

                            Type* EltTy = cast<SequentialType>(CurTy)->getElementType();
                            unsigned Size = Record.size();

                            if (EltTy->isIntegerTy(8))
                            {
                                SmallVector < uint8_t, 16 > Elts(Record.begin(), Record.end());
                                if (isa<VectorType>(CurTy))
                                    V = ConstantDataVector::get(Context, Elts);
                                else
                                    V = ConstantDataArray::get(Context, Elts);
                            }
                            else if (EltTy->isIntegerTy(16))
                            {
                                SmallVector < uint16_t, 16 > Elts(Record.begin(), Record.end());
                                if (isa<VectorType>(CurTy))
                                    V = ConstantDataVector::get(Context, Elts);
                                else
                                    V = ConstantDataArray::get(Context, Elts);
                            }
                            else if (EltTy->isIntegerTy(32))
                            {
                                SmallVector < uint32_t, 16 > Elts(Record.begin(), Record.end());
                                if (isa<VectorType>(CurTy))
                                    V = ConstantDataVector::get(Context, Elts);
                                else
                                    V = ConstantDataArray::get(Context, Elts);
                            }
                            else if (EltTy->isIntegerTy(64))
                            {
                                SmallVector < uint64_t, 16 > Elts(Record.begin(), Record.end());
                                if (isa<VectorType>(CurTy))
                                    V = ConstantDataVector::get(Context, Elts);
                                else
                                    V = ConstantDataArray::get(Context, Elts);
                            }
                            else if (EltTy->isFloatTy())
                            {
                                SmallVector < float, 16 > Elts(Size);
                                std::transform(Record.begin(), Record.end(), Elts.begin(), BitsToFloat);
                                if (isa<VectorType>(CurTy))
                                    V = ConstantDataVector::get(Context, Elts);
                                else
                                    V = ConstantDataArray::get(Context, Elts);
                            }
                            else if (EltTy->isDoubleTy())
                            {
                                SmallVector < double, 16 > Elts(Size);
                                std::transform(Record.begin(), Record.end(), Elts.begin(),
                                               BitsToDouble);
                                if (isa<VectorType>(CurTy))
                                    V = ConstantDataVector::get(Context, Elts);
                                else
                                    V = ConstantDataArray::get(Context, Elts);
                            }
                            else
                            {
                                return error("Invalid type for value");
                            }
                            break;
                        }

                    case bitc::CST_CODE_CE_BINOP:
                        {  // CE_BINOP: [opcode, opval, opval]
                            if (Record.size() < 3)
                                return error("Invalid record");
                            int Opc = getDecodedBinaryOpcode(Record[0], CurTy);
                            if (Opc < 0)
                            {
                                V = UndefValue::get(CurTy);  // Unknown binop.
                            }
                            else
                            {
                                Constant* LHS = ValueList.getConstantFwdRef(Record[1], CurTy);
                                Constant* RHS = ValueList.getConstantFwdRef(Record[2], CurTy);
                                unsigned Flags = 0;
                                if (Record.size() >= 4)
                                {
                                    if (Opc == Instruction::Add ||
                                        Opc == Instruction::Sub ||
                                        Opc == Instruction::Mul ||
                                        Opc == Instruction::Shl)
                                    {
                                        if (Record[3] & (1 << bitc::OBO_NO_SIGNED_WRAP))
                                            Flags |= OverflowingBinaryOperator::NoSignedWrap;
                                        if (Record[3] & (1 << bitc::OBO_NO_UNSIGNED_WRAP))
                                            Flags |= OverflowingBinaryOperator::NoUnsignedWrap;
                                    }
                                    else if (Opc == Instruction::SDiv ||
                                             Opc == Instruction::UDiv ||
                                             Opc == Instruction::LShr ||
                                             Opc == Instruction::AShr)
                                    {
                                        if (Record[3] & (1 << bitc::PEO_EXACT))
                                            Flags |= SDivOperator::IsExact;
                                    }
                                }
                                V = ConstantExpr::get(Opc, LHS, RHS, Flags);
                            }
                            break;
                        }
                    case bitc::CST_CODE_CE_CAST:
                        {  // CE_CAST: [opcode, opty, opval]
                            if (Record.size() < 3)
                                return error("Invalid record");
                            int Opc = getDecodedCastOpcode(Record[0]);
                            if (Opc < 0)
                            {
                                V = UndefValue::get(CurTy);  // Unknown cast.
                            }
                            else
                            {
                                Type* OpTy = getTypeByID(Record[1]);
                                if (!OpTy)
                                    return error("Invalid record");
                                Constant* Op = ValueList.getConstantFwdRef(Record[2], OpTy);
                                V = UpgradeBitCastExpr(Opc, Op, CurTy);
                                if (!V) V = ConstantExpr::getCast(Opc, Op, CurTy);
                            }
                            break;
                        }
                    case bitc::CST_CODE_CE_INBOUNDS_GEP:
                    case bitc::CST_CODE_CE_GEP:
                        {  // CE_GEP:        [n x operands]
                            unsigned OpNum = 0;
                            Type* PointeeType = nullptr;
                            if (Record.size() % 2)
                                PointeeType = getTypeByID(Record[OpNum++]);
                            SmallVector < Constant *, 16 > Elts;
                            while (OpNum != Record.size())
                            {
                                Type* ElTy = getTypeByID(Record[OpNum++]);
                                if (!ElTy)
                                    return error("Invalid record");
                                Elts.push_back(ValueList.getConstantFwdRef(Record[OpNum++], ElTy));
                            }

                            if (PointeeType &&
                                PointeeType !=
                                    cast<SequentialType>(Elts[0]->getType()->getScalarType())
                                        ->getElementType())
                                return error("Explicit gep operator type does not match pointee type "
                                             "of pointer operand");

                            ArrayRef<Constant*> Indices(Elts.begin() + 1, Elts.end());
                            V = ConstantExpr::getGetElementPtr(PointeeType, Elts[0], Indices,
                                                               BitCode ==
                                                                   bitc::CST_CODE_CE_INBOUNDS_GEP);
                            break;
                        }
                    case bitc::CST_CODE_CE_SELECT:
                        {  // CE_SELECT: [opval#, opval#, opval#]
                            if (Record.size() < 3)
                                return error("Invalid record");

                            Type* SelectorTy = Type::getInt1Ty(Context);

                            // If CurTy is a vector of length n, then Record[0] must be a <n x i1>
                            // vector. Otherwise, it must be a single bit.
                            if (VectorType * VTy = dyn_cast<VectorType>(CurTy))
                                SelectorTy = VectorType::get(Type::getInt1Ty(Context),
                                                             VTy->getNumElements());

                            V = ConstantExpr::getSelect(ValueList.getConstantFwdRef(Record[0],
                                                                                    SelectorTy),
                                                        ValueList.getConstantFwdRef(Record[1], CurTy),
                                                        ValueList.getConstantFwdRef(Record[2], CurTy));
                            break;
                        }
                    case bitc::CST_CODE_CE_EXTRACTELT
                        :
                        { // CE_EXTRACTELT: [opty, opval, opty, opval]
                            if (Record.size() < 3)
                                return error("Invalid record");
                            VectorType* OpTy =
                              dyn_cast_or_null<VectorType>(getTypeByID(Record[0]));
                            if (!OpTy)
                                return error("Invalid record");
                            Constant* Op0 = ValueList.getConstantFwdRef(Record[1], OpTy);
                            Constant* Op1 = nullptr;
                            if (Record.size() == 4)
                            {
                                Type* IdxTy = getTypeByID(Record[2]);
                                if (!IdxTy)
                                    return error("Invalid record");
                                Op1 = ValueList.getConstantFwdRef(Record[3], IdxTy);
                            }
                            else // TODO: Remove with llvm 4.0
                                Op1 = ValueList.getConstantFwdRef(Record[2], Type::getInt32Ty(Context));
                            if (!Op1)
                                return error("Invalid record");
                            V = ConstantExpr::getExtractElement(Op0, Op1);
                            break;
                        }
                    case bitc::CST_CODE_CE_INSERTELT
                        :
                        { // CE_INSERTELT: [opval, opval, opty, opval]
                            VectorType* OpTy = dyn_cast<VectorType>(CurTy);
                            if (Record.size() < 3 || !OpTy)
                                return error("Invalid record");
                            Constant* Op0 = ValueList.getConstantFwdRef(Record[0], OpTy);
                            Constant* Op1 = ValueList.getConstantFwdRef(Record[1],
                                                                        OpTy->getElementType());
                            Constant* Op2 = nullptr;
                            if (Record.size() == 4)
                            {
                                Type* IdxTy = getTypeByID(Record[2]);
                                if (!IdxTy)
                                    return error("Invalid record");
                                Op2 = ValueList.getConstantFwdRef(Record[3], IdxTy);
                            }
                            else // TODO: Remove with llvm 4.0
                                Op2 = ValueList.getConstantFwdRef(Record[2], Type::getInt32Ty(Context));
                            if (!Op2)
                                return error("Invalid record");
                            V = ConstantExpr::getInsertElement(Op0, Op1, Op2);
                            break;
                        }
                    case bitc::CST_CODE_CE_SHUFFLEVEC:
                        { // CE_SHUFFLEVEC: [opval, opval, opval]
                            VectorType* OpTy = dyn_cast<VectorType>(CurTy);
                            if (Record.size() < 3 || !OpTy)
                                return error("Invalid record");
                            Constant* Op0 = ValueList.getConstantFwdRef(Record[0], OpTy);
                            Constant* Op1 = ValueList.getConstantFwdRef(Record[1], OpTy);
                            Type* ShufTy = VectorType::get(Type::getInt32Ty(Context),
                                                                       OpTy->getNumElements());
                            Constant* Op2 = ValueList.getConstantFwdRef(Record[2], ShufTy);
                            V = ConstantExpr::getShuffleVector(Op0, Op1, Op2);
                            break;
                        }
                    case bitc::CST_CODE_CE_SHUFVEC_EX:
                        { // [opty, opval, opval, opval]
                            VectorType* RTy = dyn_cast<VectorType>(CurTy);
                            VectorType* OpTy =
                              dyn_cast_or_null<VectorType>(getTypeByID(Record[0]));
                            if (Record.size() < 4 || !RTy || !OpTy)
                                return error("Invalid record");
                            Constant* Op0 = ValueList.getConstantFwdRef(Record[1], OpTy);
                            Constant* Op1 = ValueList.getConstantFwdRef(Record[2], OpTy);
                            Type* ShufTy = VectorType::get(Type::getInt32Ty(Context),
                                                                       RTy->getNumElements());
                            Constant* Op2 = ValueList.getConstantFwdRef(Record[3], ShufTy);
                            V = ConstantExpr::getShuffleVector(Op0, Op1, Op2);
                            break;
                        }
                    case bitc::CST_CODE_CE_CMP:
                        {     // CE_CMP: [opty, opval, opval, pred]
                            if (Record.size() < 4)
                                return error("Invalid record");
                            Type* OpTy = getTypeByID(Record[0]);
                            if (!OpTy)
                                return error("Invalid record");
                            Constant* Op0 = ValueList.getConstantFwdRef(Record[1], OpTy);
                            Constant* Op1 = ValueList.getConstantFwdRef(Record[2], OpTy);

                            if (OpTy->isFPOrFPVectorTy())
                                V = ConstantExpr::getFCmp(Record[3], Op0, Op1);
                            else
                                V = ConstantExpr::getICmp(Record[3], Op0, Op1);
                            break;
                        }
                    // This maintains backward compatibility, pre-asm dialect keywords.
                    // FIXME: Remove with the 4.0 release.
                    case bitc::CST_CODE_INLINEASM_OLD:
                        {
                            if (Record.size() < 2)
                                return error("Invalid record");
                            std::string AsmStr, ConstrStr;
                            bool HasSideEffects = Record[0] & 1;
                            bool IsAlignStack = Record[0] >> 1;
                            unsigned AsmStrSize = Record[1];
                            if (2 + AsmStrSize >= Record.size())
                                return error("Invalid record");
                            unsigned ConstStrSize = Record[2 + AsmStrSize];
                            if (3 + AsmStrSize + ConstStrSize > Record.size())
                                return error("Invalid record");

                            for (unsigned i = 0; i != AsmStrSize; ++i)
                                AsmStr += (char)Record[2 + i];
                            for (unsigned i = 0; i != ConstStrSize; ++i)
                                ConstrStr += (char)Record[3 + AsmStrSize + i];
                            PointerType* PTy = cast<PointerType>(CurTy);
                            V = InlineAsm::get(cast<FunctionType>(PTy->getElementType()),
                                               AsmStr, ConstrStr, HasSideEffects, IsAlignStack);
                            break;
                        }
                    // This version adds support for the asm dialect keywords (e.g.,
                    // inteldialect).
                    case bitc::CST_CODE_INLINEASM:
                        {
                            if (Record.size() < 2)
                                return error("Invalid record");
                            std::string AsmStr, ConstrStr;
                            bool HasSideEffects = Record[0] & 1;
                            bool IsAlignStack = (Record[0] >> 1) & 1;
                            unsigned AsmDialect = Record[0] >> 2;
                            unsigned AsmStrSize = Record[1];
                            if (2 + AsmStrSize >= Record.size())
                                return error("Invalid record");
                            unsigned ConstStrSize = Record[2 + AsmStrSize];
                            if (3 + AsmStrSize + ConstStrSize > Record.size())
                                return error("Invalid record");

                            for (unsigned i = 0; i != AsmStrSize; ++i)
                                AsmStr += (char)Record[2 + i];
                            for (unsigned i = 0; i != ConstStrSize; ++i)
                                ConstrStr += (char)Record[3 + AsmStrSize + i];
                            PointerType* PTy = cast<PointerType>(CurTy);
                            V = InlineAsm::get(cast<FunctionType>(PTy->getElementType()),
                                               AsmStr, ConstrStr, HasSideEffects, IsAlignStack,
                                               InlineAsm::AsmDialect(AsmDialect));
                            break;
                        }
                    case bitc::CST_CODE_BLOCKADDRESS:
                        {
                            if (Record.size() < 3)
                                return error("Invalid record");
                            Type* FnTy = getTypeByID(Record[0]);
                            if (!FnTy)
                                return error("Invalid record");
                            Function* Fn =
                              dyn_cast_or_null<Function>(ValueList.getConstantFwdRef(Record[1], FnTy));
                            if (!Fn)
                                return error("Invalid record");

                            // Don't let Fn get dematerialized.
                            BlockAddressesTaken.insert(Fn);

                            // If the function is already parsed we can insert the block address right
                            // away.
                            BasicBlock* BB;
                            unsigned BBID = Record[2];
                            if (!BBID)
                                // Invalid reference to entry block.
                                return error("Invalid ID");
                            if (!Fn->empty())
                            {
                                Function::iterator BBI = Fn->begin(), BBE = Fn->end();
                                for (size_t I = 0, E = BBID; I != E; ++I)
                                {
                                    if (BBI == BBE)
                                        return error("Invalid ID");
                                    ++BBI;
                                }
                                BB = BBI;
                            }
                            else
                            {
                                // Otherwise insert a placeholder and remember it so it can be inserted
                                // when the function is parsed.
                                auto & FwdBBs = BasicBlockFwdRefs[Fn];
                                if (FwdBBs.empty())
                                    BasicBlockFwdRefQueue.push_back(Fn);
                                if (FwdBBs.size() < BBID + 1)
                                    FwdBBs.resize(BBID + 1);
                                if (!FwdBBs[BBID])
                                    FwdBBs[BBID] = BasicBlock::Create(Context);
                                BB = FwdBBs[BBID];
                            }
                            V = BlockAddress::get(Fn, BB);
                            break;
                        }
                }

                ValueList.assignValue(V, NextCstNo);
                ++NextCstNo;
            }
#else
                Stream.readRecord(Entry.ID, Record);
            }
#endif
        }
        private void resolveGlobalAndAliasInits()
        {
            //throw new NotImplementedException();
        }
        private void parseMetadata()
        {
            if (Stream.EnterSubBlock(METADATA_BLOCK_ID))
                throw error("Invalid record");

            List<ulong> Record = new List<ulong>();
            for (;;)
            {
                BitstreamEntry Entry = Stream.advanceSkippingSubblocks();
                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.SubBlock: // Handled for us already.
                    case BitstreamEntry.EntryKind.Error:
                        throw error("Malformed block");
                    case BitstreamEntry.EntryKind.EndBlock:
                        return;
                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read a record.
                Record.Clear();
                switch (Stream.readRecord(Entry.ID, Record))
                {
                    default:
                        break;
                }
            }
        }
        private void rememberAndSkipFunctionBody()
        {
            //throw new NotImplementedException();
        }
        private void parseUseLists()
        {
            if (Stream.EnterSubBlock(USELIST_BLOCK_ID))
                throw error("Invalid record");

            List<ulong> Record = new List<ulong>();
            for (;;)
            {
                BitstreamEntry Entry = Stream.advanceSkippingSubblocks();
                switch (Entry.Kind)
                {
                    case BitstreamEntry.EntryKind.SubBlock: // Handled for us already.
                    case BitstreamEntry.EntryKind.Error:
                        throw error("Malformed block");
                    case BitstreamEntry.EntryKind.EndBlock:
                        return;
                    case BitstreamEntry.EntryKind.Record:
                        // The interesting case.
                        break;
                }

                // Read a record.
                Record.Clear();
                switch (Stream.readRecord(Entry.ID, Record))
                {
                    default:
                        break;
                }
            }
        }
    }
}
