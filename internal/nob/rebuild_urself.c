#include "std.gob/internal/domain/api.h"
#include "std.gob/internal/nob/api.h"
#include "std.gob/internal/util.optional/api.h"
#include "std.gob/third_party/nob.h/nob.h"

DEFINE_OPTIONAL(Nob_String_View);

static Nob_String_View gob_current_build_key(void) {
#if defined(_WIN32)
  return nob_sv_from_cstr("NOB_BUILD_WINDOWS");
#elif defined(__APPLE__)
  return nob_sv_from_cstr("NOB_BUILD_MACOS");
#else
  return nob_sv_from_cstr("NOB_BUILD_LINUX");
#endif
}

static opt_Nob_String_View next_token(Nob_String_View clause, size_t start) {
  size_t i = start;
  while (i < clause.count && isspace((unsigned char)clause.data[i])) {
    i += 1;
  }
  if (i >= clause.count) { return opt_none(Nob_String_View); }

  size_t begin = i;
  bool in_single = false;
  bool in_double = false;
  bool escaped = false;
  for (; i < clause.count; ++i) {
    char c = clause.data[i];
    if (escaped) {
      escaped = false;
      continue;
    }
    if (c == '\\') {
      escaped = true;
      continue;
    }
    if (!in_double && c == '\'') {
      in_single = !in_single;
      continue;
    }
    if (!in_single && c == '"') {
      in_double = !in_double;
      continue;
    }
    if (!in_single && !in_double && isspace((unsigned char)c)) { break; }
  }

  return opt_some(Nob_String_View, nob_sv_from_parts(clause.data + begin, i - begin));
}

static opt_Nob_String_View next_sexpr_clause(Nob_String_View comment, size_t start) {
  bool in_single = false;
  bool in_double = false;
  bool escaped = false;

  size_t i = start;
  while (i < comment.count) {
    char c = comment.data[i];
    if (escaped) {
      escaped = false;
      i += 1;
      continue;
    }
    if (c == '\\') {
      escaped = true;
      i += 1;
      continue;
    }
    if (!in_double && c == '\'') {
      in_single = !in_single;
      i += 1;
      continue;
    }
    if (!in_single && c == '"') {
      in_double = !in_double;
      i += 1;
      continue;
    }
    if (!in_single && !in_double && c == '{') { break; }
    i += 1;
  }
  if (i >= comment.count) { return opt_none(Nob_String_View); }

  size_t clause_start = i;
  size_t depth = 0;
  for (; i < comment.count; ++i) {
    char c = comment.data[i];
    if (escaped) {
      escaped = false;
      continue;
    }
    if (c == '\\') {
      escaped = true;
      continue;
    }
    if (!in_double && c == '\'') {
      in_single = !in_single;
      continue;
    }
    if (!in_single && c == '"') {
      in_double = !in_double;
      continue;
    }
    if (in_single || in_double) { continue; }

    if (c == '{') {
      depth += 1;
    } else if (c == '}') {
      if (depth == 0) { return opt_none(Nob_String_View); }
      depth -= 1;
      if (depth == 0) {
        size_t len = i - clause_start + 1;
        return opt_some(Nob_String_View, nob_sv_from_parts(comment.data + clause_start, len));
      }
    }
  }

  return opt_none(Nob_String_View);
}

static opt_size_t next_include_directive_pos(Nob_String_View source, size_t start) {
  Nob_String_View cursor = nob_sv_from_parts(source.data + start, source.count - start);
  while (cursor.count > 0) {
    Nob_String_View line = nob_sv_chop_by_delim(&cursor, '\n');
    Nob_String_View trimmed = nob_sv_trim_left(line);
    if (trimmed.count == 0 || trimmed.data[0] != '#') { continue; }

    Nob_String_View after_hash = trimmed;
    nob_sv_chop_left(&after_hash, 1);
    after_hash = nob_sv_trim_left(after_hash);
    if (nob_sv_starts_with(after_hash, nob_sv_from_cstr("include"))) {
      size_t pos = (size_t)(line.data - source.data);
      return opt_some(size_t, pos);
    }
  }
  return opt_none(size_t);
}

static opt_Nob_String_View next_toplevel_block_comment(
    Nob_String_View source, size_t start, size_t end) {
  if (end > source.count) { end = source.count; }
  for (size_t i = start; i + 1 < end; ++i) {
    if (source.data[i] == '/' && source.data[i + 1] == '*') {
      size_t body_start = i + 2;
      for (size_t j = body_start; j + 1 < end; ++j) {
        if (source.data[j] == '*' && source.data[j + 1] == '/') {
          return opt_some(
              Nob_String_View,
              nob_sv_from_parts(source.data + body_start, j - body_start));
        }
      }
      return opt_none(Nob_String_View);
    }
  }
  return opt_none(Nob_String_View);
}

void gob_rebuild_from_directives(int argc, char** argv, const char* source_path) {
  int returncode = 0;
  bool defer_inputs = false, defer_cmd = false, do_return = false;
  bool defer_source = false, defer_cmd_sb = false;

  const char* binary_path = nob_shift(argv, argc);
#ifdef _WIN32
  if (!nob_sv_end_with(nob_sv_from_cstr(binary_path), ".exe")) {
    binary_path = nob_temp_sprintf("%s.exe", binary_path);
  }
#endif

  Nob_File_Paths inputs = {0};
  defer_inputs = true;
  nob_da_append(&inputs, source_path);

  int rebuild_is_needed = nob_needs_rebuild(binary_path, inputs.items, inputs.count);
  if (rebuild_is_needed < 0) {
    returncode = 1;
    goto defer;
  }
  if (!rebuild_is_needed) {
    do_return = true;
    goto defer;
  }

  const char* command = NULL;
  Nob_String_Builder command_sb = {0};
  Nob_String_Builder source = {0};
  defer_cmd_sb = true;

  if (!nob_read_entire_file(source_path, &source)) {
    returncode = 1;
    goto defer;
  }
  defer_source = true;

  Nob_String_View source_sv = nob_sv_from_parts(source.items, source.count);

  opt_size_t include_pos_opt = next_include_directive_pos(source_sv, 0);
  if (!include_pos_opt.has_value) {
    gob_log(NOB_ERROR, "failed to locate includes in %s", source_path);
    returncode = 1;
    goto defer;
  }

  opt_Nob_String_View comment_opt =
      next_toplevel_block_comment(source_sv, 0, include_pos_opt.value);
  if (!comment_opt.has_value) {
    gob_log(NOB_ERROR, "failed to locate build directives block in %s", source_path);
    returncode = 1;
    goto defer;
  }

  Nob_String_View comment = comment_opt.value;
  Nob_String_View platform_key = gob_current_build_key();

  size_t clause_cursor = 0;
  while (1) {
    opt_Nob_String_View clause_opt = next_sexpr_clause(comment, clause_cursor);
    if (!clause_opt.has_value) { break; }
    Nob_String_View clause = clause_opt.value;
    clause_cursor = (size_t)((clause.data - comment.data) + clause.count);

    opt_Nob_String_View key = opt_none(Nob_String_View);
    bool colon_found = false;

    size_t token_cursor = 0;
    while (1) {
      opt_Nob_String_View tok_opt = next_token(clause, token_cursor);
      if (!tok_opt.has_value) { break; }
      Nob_String_View tok = tok_opt.value;
      token_cursor = (size_t)((tok.data - clause.data) + tok.count);

      while (tok.count > 0 && (tok.data[0] == '{' || tok.data[0] == '}')) {
        tok.data += 1;
        tok.count -= 1;
      }
      while (tok.count > 0 && (tok.data[tok.count - 1] == '{' || tok.data[tok.count - 1] == '}')) {
        tok.count -= 1;
      }
      tok = nob_sv_trim(tok);
      if (tok.count == 0) { continue; }

      if (!colon_found && tok.data[tok.count - 1] == ':') {
        key = opt_some(Nob_String_View, nob_sv_from_parts(tok.data, tok.count - 1));
        colon_found = true;
        continue;
      }

      if (colon_found) {
        if (command_sb.count > 0) { nob_sb_append_buf(&command_sb, " ", 1); }
        nob_sb_append_buf(&command_sb, tok.data, tok.count);
      }
    }

    if (key.has_value && nob_sv_eq(key.value, platform_key) && command_sb.count > 0) {
      nob_sb_append_null(&command_sb);
      command = command_sb.items;
      break;
    }

    command_sb.count = 0;
  }

  if (command == NULL) {
    gob_log(
        NOB_ERROR,
        "no build directive for platform (%.*s) found in %s",
        (int)platform_key.count,
        platform_key.data,
        source_path);
    returncode = 1;
    goto defer;
  }

  Nob_Cmd cmd = {0};
  defer_cmd = true;
  const char* old_binary_path = nob_temp_sprintf("%s.old", binary_path);
  if (!nob_rename(binary_path, old_binary_path)) {
    returncode = 1;
    goto defer;
  }

#if defined(_WIN32)
  nob_cmd_append(&cmd, "cmd", "/C", command);
#else
  nob_cmd_append(&cmd, "sh", "-c", command);
#endif

  if (!nob_cmd_run(&cmd)) {
    nob_rename(old_binary_path, binary_path);
    returncode = 1;
    goto defer;
  }
  if (unlink(old_binary_path) < 0) {
    nob_log(
        NOB_WARNING, "could not delete temporary file %s: %s", old_binary_path, strerror(errno));
  }

  nob_cmd_append(&cmd, binary_path);
  nob_da_append_many(&cmd, argv, argc);
  if (!nob_cmd_run(&cmd)) {
    returncode = 1;
    goto defer;
  }
defer:
  if (defer_cmd_sb) { nob_sb_free(command_sb); }
  if (defer_source) { nob_sb_free(source); }
  if (defer_inputs) { NOB_FREE(inputs.items); }
  if (defer_cmd) { nob_cmd_free(cmd); }
  if (!do_return) { exit(returncode); }
}

void gob_log_impl(const char* filepath, size_t line, size_t level, const char* fmt, ...) {
  const char* level_str = "UNKNOWN";
  switch (level) {
    case NOB_ERROR: level_str = "ERROR"; break;
    case NOB_WARNING: level_str = "WARNING"; break;
    case NOB_INFO: level_str = "INFO"; break;
    default: break;
  }

  fprintf(stderr, "[%s] %s:%zu ", level_str, filepath, line);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}
