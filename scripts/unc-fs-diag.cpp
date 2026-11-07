// Prints what each std::filesystem step returns for the paths given on the
// command line, to find where a UNC path (\\server\share\...) loses its
// prefix in MinGW builds. See https://github.com/openscad/openscad/issues/5425.
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

namespace fs = std::filesystem;

static void show(const std::string& label, const fs::path& p)
{
  std::cout << "  " << label << ": string='" << p.string() << "' generic='" << p.generic_string()
            << "'\n";
}

static void err(const std::string& label, const std::error_code& ec)
{
  std::cout << "  " << label << ": ERROR " << ec.message() << "\n";
}

int main(int argc, char **argv)
{
  const fs::path original_cwd = fs::current_path();
  for (int i = 1; i < argc; ++i) {
    std::cout << "=== input: '" << argv[i] << "'\n";
    const fs::path p(argv[i]);
    show("path", p);
    std::cout << "  root_name='" << p.root_name().string() << "' root_directory='"
              << p.root_directory().string() << "' relative_path='" << p.relative_path().string()
              << "' is_absolute=" << p.is_absolute() << "\n";

    std::error_code ec;
    std::cout << "  exists=" << fs::exists(p, ec);
    if (ec) std::cout << " (ERROR " << ec.message() << ")";
    std::cout << "\n";

    ec.clear();
    const fs::path abs = fs::absolute(p, ec);
    if (ec) err("absolute", ec); else show("absolute", abs);
    const fs::path parent = abs.parent_path();
    show("absolute.parent_path", parent);

    // Round trip through generic_string(), as parser.y does.
    const fs::path round_trip(p.generic_string());
    show("path(generic_string)", round_trip);
    ec.clear();
    std::cout << "  path(generic_string) exists=" << fs::exists(round_trip, ec);
    if (ec) std::cout << " (ERROR " << ec.message() << ")";
    std::cout << "\n";

    ec.clear();
    const fs::path canon = fs::canonical(p, ec);
    if (ec) err("canonical", ec); else show("canonical", canon);

    ec.clear();
    const fs::path weak = fs::weakly_canonical(p, ec);
    if (ec) err("weakly_canonical", ec); else show("weakly_canonical", weak);

    ec.clear();
    const fs::path rel = fs::relative(p, parent, ec);
    if (ec) err("relative(path, parent)", ec); else show("relative(path, parent)", rel);

    // What do_export() does: change into the file's folder.
    ec.clear();
    fs::current_path(parent, ec);
    if (ec) {
      err("current_path(parent)", ec);
    } else {
      show("current_path(parent) ok, now", fs::current_path());
      fs::current_path(original_cwd, ec);
    }
  }
  return 0;
}
