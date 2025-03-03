//===--- SerializedDiagnosticPrinter.cpp - Serializer for diagnostics -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/SerializedDiagnosticPrinter.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Version.h"
#include "clang/Frontend/DiagnosticRenderer.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/SerializedDiagnosticReader.h"
#include "clang/Frontend/SerializedDiagnostics.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>

using namespace clang;
using namespace clang::serialized_diags;

namespace {
  
class AbbreviationMap {
  llvm::DenseMap<unsigned, unsigned> Abbrevs;
public:
  AbbreviationMap() {}
  
  void set(unsigned recordID, unsigned abbrevID) {
    assert(Abbrevs.find(recordID) == Abbrevs.end() 
           && "Abbreviation already set.");
    Abbrevs[recordID] = abbrevID;
  }
  
  unsigned get(unsigned recordID) {
    assert(Abbrevs.find(recordID) != Abbrevs.end() &&
           "Abbreviation not set.");
    return Abbrevs[recordID];
  }
};
 
typedef SmallVector<uint64_t, 64> RecordData;
typedef SmallVectorImpl<uint64_t> RecordDataImpl;

class SDiagsWriter;
  
class SDiagsRenderer : public DiagnosticNoteRenderer {
  SDiagsWriter &Writer;
public:
  SDiagsRenderer(SDiagsWriter &Writer, const LangOptions &LangOpts,
                 DiagnosticOptions *DiagOpts)
    : DiagnosticNoteRenderer(LangOpts, DiagOpts), Writer(Writer) {}

  ~SDiagsRenderer() override {}

protected:
  void emitDiagnosticMessage(SourceLocation Loc,
                             PresumedLoc PLoc,
                             DiagnosticsEngine::Level Level,
                             StringRef Message,
                             ArrayRef<CharSourceRange> Ranges,
                             const SourceManager *SM,
                             DiagOrStoredDiag D) override;

  void emitDiagnosticLoc(SourceLocation Loc, PresumedLoc PLoc,
                         DiagnosticsEngine::Level Level,
                         ArrayRef<CharSourceRange> Ranges,
                         const SourceManager &SM) override {}

  void emitNote(SourceLocation Loc, StringRef Message,
                const SourceManager *SM) override;

  void emitCodeContext(SourceLocation Loc,
                       DiagnosticsEngine::Level Level,
                       SmallVectorImpl<CharSourceRange>& Ranges,
                       ArrayRef<FixItHint> Hints,
                       const SourceManager &SM) override;

  void beginDiagnostic(DiagOrStoredDiag D,
                       DiagnosticsEngine::Level Level) override;
  void endDiagnostic(DiagOrStoredDiag D,
                     DiagnosticsEngine::Level Level) override;
};

typedef llvm::DenseMap<unsigned, unsigned> AbbrevLookup;

class SDiagsMerger : SerializedDiagnosticReader {
  SDiagsWriter &Writer;
  AbbrevLookup FileLookup;
  AbbrevLookup CategoryLookup;
  AbbrevLookup DiagFlagLookup;

public:
  SDiagsMerger(SDiagsWriter &Writer)
      : SerializedDiagnosticReader(), Writer(Writer) {}

  std::error_code mergeRecordsFromFile(const char *File) {
    return readDiagnostics(File);
  }

protected:
  std::error_code visitStartOfDiagnostic() override;
  std::error_code visitEndOfDiagnostic() override;
  std::error_code visitCategoryRecord(unsigned ID, StringRef Name) override;
  std::error_code visitDiagFlagRecord(unsigned ID, StringRef Name) override;
  std::error_code visitDiagnosticRecord(
      unsigned Severity, const serialized_diags::Location &Location,
      unsigned Category, unsigned Flag, StringRef Message) override;
  std::error_code visitFilenameRecord(unsigned ID, unsigned Size,
                                      unsigned Timestamp,
                                      StringRef Name) override;
  std::error_code visitFixitRecord(const serialized_diags::Location &Start,
                                   const serialized_diags::Location &End,
                                   StringRef CodeToInsert) override;
  std::error_code
  visitSourceRangeRecord(const serialized_diags::Location &Start,
                         const serialized_diags::Location &End) override;

private:
  std::error_code adjustSourceLocFilename(RecordData &Record,
                                          unsigned int offset);

  void adjustAbbrevID(RecordData &Record, AbbrevLookup &Lookup,
                      unsigned NewAbbrev);

  void writeRecordWithAbbrev(unsigned ID, RecordData &Record);

  void writeRecordWithBlob(unsigned ID, RecordData &Record, StringRef Blob);
};

class SDiagsWriter : public DiagnosticConsumer {
  friend class SDiagsRenderer;
  friend class SDiagsMerger;

  struct SharedState;

  explicit SDiagsWriter(IntrusiveRefCntPtr<SharedState> State)
      : LangOpts(nullptr), OriginalInstance(false), MergeChildRecords(false),
        State(State) {}

public:
  SDiagsWriter(StringRef File, DiagnosticOptions *Diags, bool MergeChildRecords)
      : LangOpts(nullptr), OriginalInstance(true),
        MergeChildRecords(MergeChildRecords),
        State(new SharedState(File, Diags)) {
    if (MergeChildRecords)
      RemoveOldDiagnostics();
    EmitPreamble();
  }

  ~SDiagsWriter() override {}

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override;

  void BeginSourceFile(const LangOptions &LO, const Preprocessor *PP) override {
    LangOpts = &LO;
  }

  void finish() override;

private:
  /// \brief Build a DiagnosticsEngine to emit diagnostics about the diagnostics
  DiagnosticsEngine *getMetaDiags();

  /// \brief Remove old copies of the serialized diagnostics. This is necessary
  /// so that we can detect when subprocesses write diagnostics that we should
  /// merge into our own.
  void RemoveOldDiagnostics();

  /// \brief Emit the preamble for the serialized diagnostics.
  void EmitPreamble();
  
  /// \brief Emit the BLOCKINFO block.
  void EmitBlockInfoBlock();

  /// \brief Emit the META data block.
  void EmitMetaBlock();

  /// \brief Start a DIAG block.
  void EnterDiagBlock();

  /// \brief End a DIAG block.
  void ExitDiagBlock();

  /// \brief Emit a DIAG record.
  void EmitDiagnosticMessage(SourceLocation Loc,
                             PresumedLoc PLoc,
                             DiagnosticsEngine::Level Level,
                             StringRef Message,
                             const SourceManager *SM,
                             DiagOrStoredDiag D);

  /// \brief Emit FIXIT and SOURCE_RANGE records for a diagnostic.
  void EmitCodeContext(SmallVectorImpl<CharSourceRange> &Ranges,
                       ArrayRef<FixItHint> Hints,
                       const SourceManager &SM);

  /// \brief Emit a record for a CharSourceRange.
  void EmitCharSourceRange(CharSourceRange R, const SourceManager &SM);
  
  /// \brief Emit the string information for the category.
  unsigned getEmitCategory(unsigned category = 0);
  
  /// \brief Emit the string information for diagnostic flags.
  unsigned getEmitDiagnosticFlag(DiagnosticsEngine::Level DiagLevel,
                                 unsigned DiagID = 0);

  unsigned getEmitDiagnosticFlag(StringRef DiagName);

  /// \brief Emit (lazily) the file string and retrieved the file identifier.
  unsigned getEmitFile(const char *Filename);

  /// \brief Add SourceLocation information the specified record.  
  void AddLocToRecord(SourceLocation Loc, const SourceManager *SM,
                      PresumedLoc PLoc, RecordDataImpl &Record,
                      unsigned TokSize = 0);

  /// \brief Add SourceLocation information the specified record.
  void AddLocToRecord(SourceLocation Loc, RecordDataImpl &Record,
                      const SourceManager *SM,
                      unsigned TokSize = 0) {
    AddLocToRecord(Loc, SM, SM ? SM->getPresumedLoc(Loc) : PresumedLoc(),
                   Record, TokSize);
  }

  /// \brief Add CharSourceRange information the specified record.
  void AddCharSourceRangeToRecord(CharSourceRange R, RecordDataImpl &Record,
                                  const SourceManager &SM);

  /// \brief Language options, which can differ from one clone of this client
  /// to another.
  const LangOptions *LangOpts;

  /// \brief Whether this is the original instance (rather than one of its
  /// clones), responsible for writing the file at the end.
  bool OriginalInstance;

  /// \brief Whether this instance should aggregate diagnostics that are
  /// generated from child processes.
  bool MergeChildRecords;

  /// \brief State that is shared among the various clones of this diagnostic
  /// consumer.
  struct SharedState : RefCountedBase<SharedState> {
    SharedState(StringRef File, DiagnosticOptions *Diags)
        : DiagOpts(Diags), Stream(Buffer), OutputFile(File.str()),
          EmittedAnyDiagBlocks(false) {}

    /// \brief Diagnostic options.
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;

    /// \brief The byte buffer for the serialized content.
    SmallString<1024> Buffer;

    /// \brief The BitStreamWriter for the serialized diagnostics.
    llvm::BitstreamWriter Stream;

    /// \brief The name of the diagnostics file.
    std::string OutputFile;

    /// \brief The set of constructed record abbreviations.
    AbbreviationMap Abbrevs;

    /// \brief A utility buffer for constructing record content.
    RecordData Record;

    /// \brief A text buffer for rendering diagnostic text.
    SmallString<256> diagBuf;

    /// \brief The collection of diagnostic categories used.
    llvm::DenseSet<unsigned> Categories;

    /// \brief The collection of files used.
    llvm::DenseMap<const char *, unsigned> Files;

    typedef llvm::DenseMap<const void *, std::pair<unsigned, StringRef> >
    DiagFlagsTy;

    /// \brief Map for uniquing strings.
    DiagFlagsTy DiagFlags;

    /// \brief Whether we have already started emission of any DIAG blocks. Once
    /// this becomes \c true, we never close a DIAG block until we know that we're
    /// starting another one or we're done.
    bool EmittedAnyDiagBlocks;

    /// \brief Engine for emitting diagnostics about the diagnostics.
    std::unique_ptr<DiagnosticsEngine> MetaDiagnostics;
  };

  /// \brief State shared among the various clones of this diagnostic consumer.
  IntrusiveRefCntPtr<SharedState> State;
};
} // end anonymous namespace

namespace clang {
namespace serialized_diags {
std::unique_ptr<DiagnosticConsumer>
create(StringRef OutputFile, DiagnosticOptions *Diags, bool MergeChildRecords) {
  return llvm::make_unique<SDiagsWriter>(OutputFile, Diags, MergeChildRecords);
}

} // end namespace serialized_diags
} // end namespace clang

//===----------------------------------------------------------------------===//
// Serialization methods.
//===----------------------------------------------------------------------===//

/// \brief Emits a block ID in the BLOCKINFO block.
static void EmitBlockID(unsigned ID, const char *Name,
                        llvm::BitstreamWriter &Stream,
                        RecordDataImpl &Record) {
  Record.clear();
  Record.push_back(ID);
  Stream.EmitRecord(llvm::bitc::BLOCKINFO_CODE_SETBID, Record);
  
  // Emit the block name if present.
  if (!Name || Name[0] == 0)
    return;

  Record.clear();

  while (*Name)
    Record.push_back(*Name++);

  Stream.EmitRecord(llvm::bitc::BLOCKINFO_CODE_BLOCKNAME, Record);
}

/// \brief Emits a record ID in the BLOCKINFO block.
static void EmitRecordID(unsigned ID, const char *Name,
                         llvm::BitstreamWriter &Stream,
                         RecordDataImpl &Record){
  Record.clear();
  Record.push_back(ID);

  while (*Name)
    Record.push_back(*Name++);

  Stream.EmitRecord(llvm::bitc::BLOCKINFO_CODE_SETRECORDNAME, Record);
}

void SDiagsWriter::AddLocToRecord(SourceLocation Loc,
                                  const SourceManager *SM,
                                  PresumedLoc PLoc,
                                  RecordDataImpl &Record,
                                  unsigned TokSize) {
  if (PLoc.isInvalid()) {
    // Emit a "sentinel" location.
    Record.push_back((unsigned)0); // File.
    Record.push_back((unsigned)0); // Line.
    Record.push_back((unsigned)0); // Column.
    Record.push_back((unsigned)0); // Offset.
    return;
  }

  Record.push_back(getEmitFile(PLoc.getFilename()));
  Record.push_back(PLoc.getLine());
  Record.push_back(PLoc.getColumn()+TokSize);
  Record.push_back(SM->getFileOffset(Loc));
}

void SDiagsWriter::AddCharSourceRangeToRecord(CharSourceRange Range,
                                              RecordDataImpl &Record,
                                              const SourceManager &SM) {
  AddLocToRecord(Range.getBegin(), Record, &SM);
  unsigned TokSize = 0;
  if (Range.isTokenRange())
    TokSize = Lexer::MeasureTokenLength(Range.getEnd(),
                                        SM, *LangOpts);
  
  AddLocToRecord(Range.getEnd(), Record, &SM, TokSize);
}

unsigned SDiagsWriter::getEmitFile(const char *FileName){
  if (!FileName)
    return 0;
  
  unsigned &entry = State->Files[FileName];
  if (entry)
    return entry;
  
  // Lazily generate the record for the file.
  entry = State->Files.size();
  RecordData Record;
  Record.push_back(RECORD_FILENAME);
  Record.push_back(entry);
  Record.push_back(0); // For legacy.
  Record.push_back(0); // For legacy.
  StringRef Name(FileName);
  Record.push_back(Name.size());
  State->Stream.EmitRecordWithBlob(State->Abbrevs.get(RECORD_FILENAME), Record,
                                   Name);

  return entry;
}

void SDiagsWriter::EmitCharSourceRange(CharSourceRange R,
                                       const SourceManager &SM) {
  State->Record.clear();
  State->Record.push_back(RECORD_SOURCE_RANGE);
  AddCharSourceRangeToRecord(R, State->Record, SM);
  State->Stream.EmitRecordWithAbbrev(State->Abbrevs.get(RECORD_SOURCE_RANGE),
                                     State->Record);
}

/// \brief Emits the preamble of the diagnostics file.
void SDiagsWriter::EmitPreamble() {
  // Emit the file header.
  State->Stream.Emit((unsigned)'D', 8);
  State->Stream.Emit((unsigned)'I', 8);
  State->Stream.Emit((unsigned)'A', 8);
  State->Stream.Emit((unsigned)'G', 8);

  EmitBlockInfoBlock();
  EmitMetaBlock();
}

static void AddSourceLocationAbbrev(llvm::BitCodeAbbrev *Abbrev) {
  using namespace llvm;
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 10)); // File ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Line.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Column.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Offset;
}

static void AddRangeLocationAbbrev(llvm::BitCodeAbbrev *Abbrev) {
  AddSourceLocationAbbrev(Abbrev);
  AddSourceLocationAbbrev(Abbrev);  
}

void SDiagsWriter::EmitBlockInfoBlock() {
  State->Stream.EnterBlockInfoBlock(3);

  using namespace llvm;
  llvm::BitstreamWriter &Stream = State->Stream;
  RecordData &Record = State->Record;
  AbbreviationMap &Abbrevs = State->Abbrevs;

  // ==---------------------------------------------------------------------==//
  // The subsequent records and Abbrevs are for the "Meta" block.
  // ==---------------------------------------------------------------------==//

  EmitBlockID(BLOCK_META, "Meta", Stream, Record);
  EmitRecordID(RECORD_VERSION, "Version", Stream, Record);
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_VERSION));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
  Abbrevs.set(RECORD_VERSION, Stream.EmitBlockInfoAbbrev(BLOCK_META, Abbrev));

  // ==---------------------------------------------------------------------==//
  // The subsequent records and Abbrevs are for the "Diagnostic" block.
  // ==---------------------------------------------------------------------==//

  EmitBlockID(BLOCK_DIAG, "Diag", Stream, Record);
  EmitRecordID(RECORD_DIAG, "DiagInfo", Stream, Record);
  EmitRecordID(RECORD_SOURCE_RANGE, "SrcRange", Stream, Record);
  EmitRecordID(RECORD_CATEGORY, "CatName", Stream, Record);
  EmitRecordID(RECORD_DIAG_FLAG, "DiagFlag", Stream, Record);
  EmitRecordID(RECORD_FILENAME, "FileName", Stream, Record);
  EmitRecordID(RECORD_FIXIT, "FixIt", Stream, Record);

  // Emit abbreviation for RECORD_DIAG.
  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_DIAG));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 3));  // Diag level.
  AddSourceLocationAbbrev(Abbrev);
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 10)); // Category.  
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 10)); // Mapped Diag ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Diagnostc text.
  Abbrevs.set(RECORD_DIAG, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG, Abbrev));
  
  // Emit abbrevation for RECORD_CATEGORY.
  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_CATEGORY));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Category ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 8));  // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));      // Category text.
  Abbrevs.set(RECORD_CATEGORY, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG, Abbrev));

  // Emit abbrevation for RECORD_SOURCE_RANGE.
  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_SOURCE_RANGE));
  AddRangeLocationAbbrev(Abbrev);
  Abbrevs.set(RECORD_SOURCE_RANGE,
              Stream.EmitBlockInfoAbbrev(BLOCK_DIAG, Abbrev));
  
  // Emit the abbreviation for RECORD_DIAG_FLAG.
  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_DIAG_FLAG));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 10)); // Mapped Diag ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Flag name text.
  Abbrevs.set(RECORD_DIAG_FLAG, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG,
                                                           Abbrev));
  
  // Emit the abbreviation for RECORD_FILENAME.
  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_FILENAME));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 10)); // Mapped file ID.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // Modifcation time.  
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // File name text.
  Abbrevs.set(RECORD_FILENAME, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG,
                                                          Abbrev));
  
  // Emit the abbreviation for RECORD_FIXIT.
  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(RECORD_FIXIT));
  AddRangeLocationAbbrev(Abbrev);
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Text size.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));      // FixIt text.
  Abbrevs.set(RECORD_FIXIT, Stream.EmitBlockInfoAbbrev(BLOCK_DIAG,
                                                       Abbrev));

  Stream.ExitBlock();
}

void SDiagsWriter::EmitMetaBlock() {
  llvm::BitstreamWriter &Stream = State->Stream;
  RecordData &Record = State->Record;
  AbbreviationMap &Abbrevs = State->Abbrevs;

  Stream.EnterSubblock(BLOCK_META, 3);
  Record.clear();
  Record.push_back(RECORD_VERSION);
  Record.push_back(VersionNumber);
  Stream.EmitRecordWithAbbrev(Abbrevs.get(RECORD_VERSION), Record);  
  Stream.ExitBlock();
}

unsigned SDiagsWriter::getEmitCategory(unsigned int category) {
  if (!State->Categories.insert(category).second)
    return category;

  // We use a local version of 'Record' so that we can be generating
  // another record when we lazily generate one for the category entry.
  RecordData Record;
  Record.push_back(RECORD_CATEGORY);
  Record.push_back(category);
  StringRef catName = DiagnosticIDs::getCategoryNameFromID(category);
  Record.push_back(catName.size());
  State->Stream.EmitRecordWithBlob(State->Abbrevs.get(RECORD_CATEGORY), Record,
                                   catName);
  
  return category;
}

unsigned SDiagsWriter::getEmitDiagnosticFlag(DiagnosticsEngine::Level DiagLevel,
                                             unsigned DiagID) {
  if (DiagLevel == DiagnosticsEngine::Note)
    return 0; // No flag for notes.
  
  StringRef FlagName = DiagnosticIDs::getWarningOptionForDiag(DiagID);
  return getEmitDiagnosticFlag(FlagName);
}

unsigned SDiagsWriter::getEmitDiagnosticFlag(StringRef FlagName) {
  if (FlagName.empty())
    return 0;

  // Here we assume that FlagName points to static data whose pointer
  // value is fixed.  This allows us to unique by diagnostic groups.
  const void *data = FlagName.data();
  std::pair<unsigned, StringRef> &entry = State->DiagFlags[data];
  if (entry.first == 0) {
    entry.first = State->DiagFlags.size();
    entry.second = FlagName;
    
    // Lazily emit the string in a separate record.
    RecordData Record;
    Record.push_back(RECORD_DIAG_FLAG);
    Record.push_back(entry.first);
    Record.push_back(FlagName.size());
    State->Stream.EmitRecordWithBlob(State->Abbrevs.get(RECORD_DIAG_FLAG),
                                     Record, FlagName);
  }

  return entry.first;
}

void SDiagsWriter::HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                                    const Diagnostic &Info) {
  // Enter the block for a non-note diagnostic immediately, rather than waiting
  // for beginDiagnostic, in case associated notes are emitted before we get
  // there.
  if (DiagLevel != DiagnosticsEngine::Note) {
    if (State->EmittedAnyDiagBlocks)
      ExitDiagBlock();

    EnterDiagBlock();
    State->EmittedAnyDiagBlocks = true;
  }

  // Compute the diagnostic text.
  State->diagBuf.clear();
  Info.FormatDiagnostic(State->diagBuf);

  if (Info.getLocation().isInvalid()) {
    // Special-case diagnostics with no location. We may not have entered a
    // source file in this case, so we can't use the normal DiagnosticsRenderer
    // machinery.

    // Make sure we bracket all notes as "sub-diagnostics".  This matches
    // the behavior in SDiagsRenderer::emitDiagnostic().
    if (DiagLevel == DiagnosticsEngine::Note)
      EnterDiagBlock();

    EmitDiagnosticMessage(SourceLocation(), PresumedLoc(), DiagLevel,
                          State->diagBuf, nullptr, &Info);

    if (DiagLevel == DiagnosticsEngine::Note)
      ExitDiagBlock();

    return;
  }

  assert(Info.hasSourceManager() && LangOpts &&
         "Unexpected diagnostic with valid location outside of a source file");
  SDiagsRenderer Renderer(*this, *LangOpts, &*State->DiagOpts);
  Renderer.emitDiagnostic(Info.getLocation(), DiagLevel,
                          State->diagBuf,
                          Info.getRanges(),
                          Info.getFixItHints(),
                          &Info.getSourceManager(),
                          &Info);
}

static serialized_diags::Level getStableLevel(DiagnosticsEngine::Level Level) {
  switch (Level) {
#define CASE(X) case DiagnosticsEngine::X: return serialized_diags::X;
  CASE(Ignored)
  CASE(Note)
  CASE(Remark)
  CASE(Warning)
  CASE(Error)
  CASE(Fatal)
#undef CASE
  }

  llvm_unreachable("invalid diagnostic level");
}

void SDiagsWriter::EmitDiagnosticMessage(SourceLocation Loc,
                                         PresumedLoc PLoc,
                                         DiagnosticsEngine::Level Level,
                                         StringRef Message,
                                         const SourceManager *SM,
                                         DiagOrStoredDiag D) {
  llvm::BitstreamWriter &Stream = State->Stream;
  RecordData &Record = State->Record;
  AbbreviationMap &Abbrevs = State->Abbrevs;
  
  // Emit the RECORD_DIAG record.
  Record.clear();
  Record.push_back(RECORD_DIAG);
  Record.push_back(getStableLevel(Level));
  AddLocToRecord(Loc, SM, PLoc, Record);

  if (const Diagnostic *Info = D.dyn_cast<const Diagnostic*>()) {
    // Emit the category string lazily and get the category ID.
    unsigned DiagID = DiagnosticIDs::getCategoryNumberForDiag(Info->getID());
    Record.push_back(getEmitCategory(DiagID));
    // Emit the diagnostic flag string lazily and get the mapped ID.
    Record.push_back(getEmitDiagnosticFlag(Level, Info->getID()));
  } else {
    Record.push_back(getEmitCategory());
    Record.push_back(getEmitDiagnosticFlag(Level));
  }

  Record.push_back(Message.size());
  Stream.EmitRecordWithBlob(Abbrevs.get(RECORD_DIAG), Record, Message);
}

void
SDiagsRenderer::emitDiagnosticMessage(SourceLocation Loc,
                                      PresumedLoc PLoc,
                                      DiagnosticsEngine::Level Level,
                                      StringRef Message,
                                      ArrayRef<clang::CharSourceRange> Ranges,
                                      const SourceManager *SM,
                                      DiagOrStoredDiag D) {
  Writer.EmitDiagnosticMessage(Loc, PLoc, Level, Message, SM, D);
}

void SDiagsWriter::EnterDiagBlock() {
  State->Stream.EnterSubblock(BLOCK_DIAG, 4);
}

void SDiagsWriter::ExitDiagBlock() {
  State->Stream.ExitBlock();
}

void SDiagsRenderer::beginDiagnostic(DiagOrStoredDiag D,
                                     DiagnosticsEngine::Level Level) {
  if (Level == DiagnosticsEngine::Note)
    Writer.EnterDiagBlock();
}

void SDiagsRenderer::endDiagnostic(DiagOrStoredDiag D,
                                   DiagnosticsEngine::Level Level) {
  // Only end note diagnostics here, because we can't be sure when we've seen
  // the last note associated with a non-note diagnostic.
  if (Level == DiagnosticsEngine::Note)
    Writer.ExitDiagBlock();
}

void SDiagsWriter::EmitCodeContext(SmallVectorImpl<CharSourceRange> &Ranges,
                                   ArrayRef<FixItHint> Hints,
                                   const SourceManager &SM) {
  llvm::BitstreamWriter &Stream = State->Stream;
  RecordData &Record = State->Record;
  AbbreviationMap &Abbrevs = State->Abbrevs;

  // Emit Source Ranges.
  for (ArrayRef<CharSourceRange>::iterator I = Ranges.begin(), E = Ranges.end();
       I != E; ++I)
    if (I->isValid())
      EmitCharSourceRange(*I, SM);

  // Emit FixIts.
  for (ArrayRef<FixItHint>::iterator I = Hints.begin(), E = Hints.end();
       I != E; ++I) {
    const FixItHint &Fix = *I;
    if (Fix.isNull())
      continue;
    Record.clear();
    Record.push_back(RECORD_FIXIT);
    AddCharSourceRangeToRecord(Fix.RemoveRange, Record, SM);
    Record.push_back(Fix.CodeToInsert.size());
    Stream.EmitRecordWithBlob(Abbrevs.get(RECORD_FIXIT), Record,
                              Fix.CodeToInsert);
  }
}

void SDiagsRenderer::emitCodeContext(SourceLocation Loc,
                                     DiagnosticsEngine::Level Level,
                                     SmallVectorImpl<CharSourceRange> &Ranges,
                                     ArrayRef<FixItHint> Hints,
                                     const SourceManager &SM) {
  Writer.EmitCodeContext(Ranges, Hints, SM);
}

void SDiagsRenderer::emitNote(SourceLocation Loc, StringRef Message,
                              const SourceManager *SM) {
  Writer.EnterDiagBlock();
  PresumedLoc PLoc = SM ? SM->getPresumedLoc(Loc) : PresumedLoc();
  Writer.EmitDiagnosticMessage(Loc, PLoc, DiagnosticsEngine::Note,
                               Message, SM, DiagOrStoredDiag());
  Writer.ExitDiagBlock();
}

DiagnosticsEngine *SDiagsWriter::getMetaDiags() {
  // FIXME: It's slightly absurd to create a new diagnostics engine here, but
  // the other options that are available today are worse:
  //
  // 1. Teach DiagnosticsConsumers to emit diagnostics to the engine they are a
  //    part of. The DiagnosticsEngine would need to know not to send
  //    diagnostics back to the consumer that failed. This would require us to
  //    rework ChainedDiagnosticsConsumer and teach the engine about multiple
  //    consumers, which is difficult today because most APIs interface with
  //    consumers rather than the engine itself.
  //
  // 2. Pass a DiagnosticsEngine to SDiagsWriter on creation - this would need
  //    to be distinct from the engine the writer was being added to and would
  //    normally not be used.
  if (!State->MetaDiagnostics) {
    IntrusiveRefCntPtr<DiagnosticIDs> IDs(new DiagnosticIDs());
    auto Client =
        new TextDiagnosticPrinter(llvm::errs(), State->DiagOpts.get());
    State->MetaDiagnostics = llvm::make_unique<DiagnosticsEngine>(
        IDs, State->DiagOpts.get(), Client);
  }
  return State->MetaDiagnostics.get();
}

void SDiagsWriter::RemoveOldDiagnostics() {
  if (!llvm::sys::fs::remove(State->OutputFile))
    return;

  getMetaDiags()->Report(diag::warn_fe_serialized_diag_merge_failure);
  // Disable merging child records, as whatever is in this file may be
  // misleading.
  MergeChildRecords = false;
}

void SDiagsWriter::finish() {
  // The original instance is responsible for writing the file.
  if (!OriginalInstance)
    return;

  // Finish off any diagnostic we were in the process of emitting.
  if (State->EmittedAnyDiagBlocks)
    ExitDiagBlock();

  if (MergeChildRecords) {
    if (!State->EmittedAnyDiagBlocks)
      // We have no diagnostics of our own, so we can just leave the child
      // process' output alone
      return;

    if (llvm::sys::fs::exists(State->OutputFile))
      if (SDiagsMerger(*this).mergeRecordsFromFile(State->OutputFile.c_str()))
        getMetaDiags()->Report(diag::warn_fe_serialized_diag_merge_failure);
  }

  std::error_code EC;
  auto OS = llvm::make_unique<llvm::raw_fd_ostream>(State->OutputFile.c_str(),
                                                    EC, llvm::sys::fs::F_None);
  if (EC) {
    getMetaDiags()->Report(diag::warn_fe_serialized_diag_failure)
        << State->OutputFile << EC.message();
    return;
  }

  // Write the generated bitstream to "Out".
  OS->write((char *)&State->Buffer.front(), State->Buffer.size());
  OS->flush();
}

std::error_code SDiagsMerger::visitStartOfDiagnostic() {
  Writer.EnterDiagBlock();
  return std::error_code();
}

std::error_code SDiagsMerger::visitEndOfDiagnostic() {
  Writer.ExitDiagBlock();
  return std::error_code();
}

std::error_code
SDiagsMerger::visitSourceRangeRecord(const serialized_diags::Location &Start,
                                     const serialized_diags::Location &End) {
  RecordData Record;
  Record.push_back(RECORD_SOURCE_RANGE);
  Record.push_back(FileLookup[Start.FileID]);
  Record.push_back(Start.Line);
  Record.push_back(Start.Col);
  Record.push_back(Start.Offset);
  Record.push_back(FileLookup[End.FileID]);
  Record.push_back(End.Line);
  Record.push_back(End.Col);
  Record.push_back(End.Offset);

  Writer.State->Stream.EmitRecordWithAbbrev(
      Writer.State->Abbrevs.get(RECORD_SOURCE_RANGE), Record);
  return std::error_code();
}

std::error_code SDiagsMerger::visitDiagnosticRecord(
    unsigned Severity, const serialized_diags::Location &Location,
    unsigned Category, unsigned Flag, StringRef Message) {
  RecordData MergedRecord;
  MergedRecord.push_back(RECORD_DIAG);
  MergedRecord.push_back(Severity);
  MergedRecord.push_back(FileLookup[Location.FileID]);
  MergedRecord.push_back(Location.Line);
  MergedRecord.push_back(Location.Col);
  MergedRecord.push_back(Location.Offset);
  MergedRecord.push_back(CategoryLookup[Category]);
  MergedRecord.push_back(Flag ? DiagFlagLookup[Flag] : 0);
  MergedRecord.push_back(Message.size());

  Writer.State->Stream.EmitRecordWithBlob(
      Writer.State->Abbrevs.get(RECORD_DIAG), MergedRecord, Message);
  return std::error_code();
}

std::error_code
SDiagsMerger::visitFixitRecord(const serialized_diags::Location &Start,
                               const serialized_diags::Location &End,
                               StringRef Text) {
  RecordData Record;
  Record.push_back(RECORD_FIXIT);
  Record.push_back(FileLookup[Start.FileID]);
  Record.push_back(Start.Line);
  Record.push_back(Start.Col);
  Record.push_back(Start.Offset);
  Record.push_back(FileLookup[End.FileID]);
  Record.push_back(End.Line);
  Record.push_back(End.Col);
  Record.push_back(End.Offset);
  Record.push_back(Text.size());

  Writer.State->Stream.EmitRecordWithBlob(
      Writer.State->Abbrevs.get(RECORD_FIXIT), Record, Text);
  return std::error_code();
}

std::error_code SDiagsMerger::visitFilenameRecord(unsigned ID, unsigned Size,
                                                  unsigned Timestamp,
                                                  StringRef Name) {
  FileLookup[ID] = Writer.getEmitFile(Name.str().c_str());
  return std::error_code();
}

std::error_code SDiagsMerger::visitCategoryRecord(unsigned ID, StringRef Name) {
  CategoryLookup[ID] = Writer.getEmitCategory(ID);
  return std::error_code();
}

std::error_code SDiagsMerger::visitDiagFlagRecord(unsigned ID, StringRef Name) {
  DiagFlagLookup[ID] = Writer.getEmitDiagnosticFlag(Name);
  return std::error_code();
}
