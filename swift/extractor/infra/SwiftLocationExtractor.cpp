#include <swift/AST/Expr.h>
#include <swift/AST/SourceFile.h>
#include <swift/Basic/SourceManager.h>
#include <swift/Parse/Token.h>

#include "swift/extractor/trap/TrapDomain.h"
#include "swift/extractor/trap/generated/TrapEntries.h"
#include "swift/extractor/trap/generated/TrapClasses.h"
#include "swift/extractor/infra/SwiftLocationExtractor.h"
#include "swift/extractor/infra/file/Path.h"
#include "swift/extractor/infra/SwiftMangledName.h"

using namespace codeql;

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                swift::SourceLoc start,
                                                swift::SourceLoc end,
                                                TrapLabel<LocatableTag> locatableLabel) {
  if (!start.isValid() || !end.isValid()) {
    // invalid locations seem to come from entities synthesized by the compiler
    return;
  }
  auto file = resolvePath(sourceManager.getDisplayNameForLoc(start));
  DbLocation entry{{}};
  entry.file = fetchFileLabel(file);
  std::tie(entry.start_line, entry.start_column) = sourceManager.getLineAndColumnInBuffer(start);
  std::tie(entry.end_line, entry.end_column) = sourceManager.getLineAndColumnInBuffer(end);
  SwiftMangledName locName{{"loc", entry.file, ":", entry.start_line, ":", entry.start_column, ":",
                            entry.end_line, ":", entry.end_column}};
  entry.id = trap.createTypedLabel<DbLocationTag>(locName);
  trap.emit(entry);
  trap.emit(LocatableLocationsTrap{locatableLabel, entry.id});
}

TrapLabel<FileTag> SwiftLocationExtractor::emitFile(swift::SourceFile* file) {
  if (file) {
    return emitFile(std::string_view{file->getFilename()});
  }
  return undefined_label;
}

TrapLabel<FileTag> SwiftLocationExtractor::emitFile(const std::filesystem::path& file) {
  return fetchFileLabel(resolvePath(file));
}

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                const swift::SourceRange& range,
                                                TrapLabel<LocatableTag> locatableLabel) {
  attachLocationImpl(sourceManager, range.Start, range.End, locatableLabel);
}

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                const swift::CapturedValue* capture,
                                                TrapLabel<LocatableTag> locatableLabel) {
  attachLocationImpl(sourceManager, capture->getLoc(), locatableLabel);
}

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                const swift::IfConfigClause* clause,
                                                TrapLabel<LocatableTag> locatableLabel) {
  attachLocationImpl(sourceManager, clause->Loc, locatableLabel);
}

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                const swift::AvailabilitySpec* spec,
                                                TrapLabel<LocatableTag> locatableLabel) {
  attachLocationImpl(sourceManager, spec->getSourceRange(), locatableLabel);
}

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                const swift::KeyPathExpr::Component* component,
                                                TrapLabel<LocatableTag> locatableLabel) {
  attachLocationImpl(sourceManager, component->getSourceRange().Start,
                     component->getSourceRange().End, locatableLabel);
}

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                const swift::Token* token,
                                                TrapLabel<LocatableTag> locatableLabel) {
  attachLocationImpl(sourceManager, token->getRange().getStart(), token->getRange().getEnd(),
                     locatableLabel);
}

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                swift::SourceLoc loc,
                                                TrapLabel<LocatableTag> locatableLabel) {
  attachLocationImpl(sourceManager, loc, loc, locatableLabel);
}

void SwiftLocationExtractor::attachLocationImpl(const swift::SourceManager& sourceManager,
                                                const swift::DiagnosticInfo* diagInfo,
                                                TrapLabel<LocatableTag> locatableLabel) {
  attachLocationImpl(sourceManager, diagInfo->Loc, locatableLabel);
}

TrapLabel<FileTag> SwiftLocationExtractor::fetchFileLabel(const std::filesystem::path& file) {
  if (store.count(file)) {
    return store[file];
  }

  DbFile entry({});
  entry.id = trap.createTypedLabel<DbFileTag>({{"file_", file.string()}});
  entry.name = file.string();
  trap.emit(entry);
  store[file] = entry.id;
  return entry.id;
}
