#pragma once

#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rpg {
namespace ui {

using namespace ftxui;

struct ExperienceSettings {
  int palette = 0;
  int text_color = -1;
  int accent_color = -1;
  int navigation_keys = 0;
  bool reduced_motion = false;
  bool compact_ui = false;
  bool detailed_combat_log = true;
};

inline std::filesystem::path settings_file_path() {
  if (const char *config_home = std::getenv("XDG_CONFIG_HOME");
      config_home != nullptr && config_home[0] != '\0') {
    return std::filesystem::path(config_home) / "rpg-enhanced" / "settings.conf";
  }
  if (const char *home = std::getenv("HOME"); home != nullptr && home[0] != '\0') {
    return std::filesystem::path(home) / ".config" / "rpg-enhanced" /
           "settings.conf";
  }
  return {};
}

inline ExperienceSettings load_settings() {
  ExperienceSettings result;
  std::ifstream file(settings_file_path());
  if (!file) {
    return result;
  }

  std::unordered_map<std::string, int> values;
  std::string line;
  while (std::getline(file, line)) {
    const std::size_t separator = line.find('=');
    if (separator == std::string::npos) {
      continue;
    }
    try {
      values[line.substr(0, separator)] =
          std::stoi(line.substr(separator + 1));
    } catch (const std::exception &) {
    }
  }
  const auto read = [&values](const std::string &key, int fallback, int minimum,
                              int maximum) {
    const auto value = values.find(key);
    if (value == values.end() || value->second < minimum ||
        value->second > maximum) {
      return fallback;
    }
    return value->second;
  };
  result.palette = read("palette", result.palette, 0, 2);
  result.text_color = read("text_color", result.text_color, -1, 3);
  result.accent_color = read("accent_color", result.accent_color, -1, 3);
  result.navigation_keys = read("navigation_keys", result.navigation_keys, 0, 2);
  result.reduced_motion = read("reduced_motion", 0, 0, 1) != 0;
  result.compact_ui = read("compact_ui", 0, 0, 1) != 0;
  result.detailed_combat_log = read("detailed_combat_log", 1, 0, 1) != 0;
  return result;
}

inline ExperienceSettings &settings() {
  static ExperienceSettings current = load_settings();
  return current;
}

inline void save_settings() {
  const std::filesystem::path path = settings_file_path();
  if (path.empty()) {
    return;
  }
  std::error_code error;
  std::filesystem::create_directories(path.parent_path(), error);
  if (error) {
    return;
  }
  std::ofstream file(path);
  if (!file) {
    return;
  }
  const ExperienceSettings &current = settings();
  file << "palette=" << current.palette << '\n'
       << "text_color=" << current.text_color << '\n'
       << "accent_color=" << current.accent_color << '\n'
       << "navigation_keys=" << current.navigation_keys << '\n'
       << "reduced_motion=" << current.reduced_motion << '\n'
      << "compact_ui=" << current.compact_ui << '\n'
      << "detailed_combat_log=" << current.detailed_combat_log << '\n';
}

struct ColorPalette {
  std::string name;
  Color text;
  Color accent;
  Color success;
  Color warning;
  Color error;
  Color health;
  Color mana;
  Color wealth;
};

inline const std::vector<ColorPalette> &color_palettes() {
  static const std::vector<ColorPalette> palettes{
      {"Standard", Color::White, Color::RGB(91, 205, 183),
       Color::RGB(91, 205, 183), Color::RGB(232, 166, 76),
       Color::RGB(224, 95, 91), Color::Green, Color::BlueViolet, Color::Gold1},
      {"High contrast", Color::White, Color::RGB(255, 220, 90),
       Color::RGB(100, 255, 120), Color::RGB(255, 220, 90),
       Color::RGB(255, 100, 100), Color::RGB(100, 255, 120),
       Color::RGB(100, 220, 255), Color::RGB(255, 220, 90)},
      {"Color-blind friendly", Color::White, Color::RGB(86, 180, 233),
       Color::RGB(86, 180, 233), Color::RGB(230, 159, 0),
       Color::RGB(213, 94, 0), Color::RGB(0, 158, 150),
       Color::RGB(86, 180, 233), Color::RGB(240, 228, 66)},
  };
  return palettes;
}

inline const ColorPalette &color_palette() {
  return color_palettes()[static_cast<std::size_t>(settings().palette)];
}

inline const std::vector<std::pair<std::string, Color>> &text_colors() {
  static const std::vector<std::pair<std::string, Color>> colors{
      {"White", Color::White},
      {"Light cyan", Color::RGB(150, 225, 230)},
      {"Warm yellow", Color::RGB(255, 220, 150)},
      {"Light green", Color::RGB(175, 230, 175)},
  };
  return colors;
}

inline const std::vector<std::pair<std::string, Color>> &accent_colors() {
  static const std::vector<std::pair<std::string, Color>> colors{
      {"Teal", Color::RGB(91, 205, 183)},
      {"Amber", Color::RGB(232, 166, 76)},
      {"Rose", Color::RGB(224, 95, 91)},
      {"Sky blue", Color::RGB(113, 170, 230)},
  };
  return colors;
}

inline Color text_color() {
  return settings().text_color < 0
             ? color_palette().text
             : text_colors()[static_cast<std::size_t>(settings().text_color)].second;
}

inline Color accent_color() {
  return settings().accent_color < 0
             ? color_palette().accent
             : accent_colors()[static_cast<std::size_t>(settings().accent_color)].second;
}

inline Color success_color() { return color_palette().success; }
inline Color warning_color() { return color_palette().warning; }
inline Color error_color() { return color_palette().error; }
inline Color health_color() { return color_palette().health; }
inline Color mana_color() { return color_palette().mana; }
inline Color wealth_color() { return color_palette().wealth; }

inline Element spacer() {
  return text(settings().compact_ui ? "" : " ") |
         size(HEIGHT, EQUAL, settings().compact_ui ? 0 : 1);
}

inline bool handle_navigation_key(const Event &event, const Component &target) {
  if (!event.is_character() || event.character().size() != 1) {
    return false;
  }
  const char key = event.character()[0];
  const bool wasd_key = key == 'w' || key == 'a' || key == 's' || key == 'd';
  const bool vim_key = key == 'h' || key == 'j' || key == 'k' || key == 'l';
  if (!wasd_key && !vim_key) {
    return false;
  }

  Event mapped;
  if (settings().navigation_keys == 1 && wasd_key) {
    mapped = key == 'w' ? Event::ArrowUp
             : key == 's' ? Event::ArrowDown
             : key == 'a' ? Event::ArrowLeft
                          : Event::ArrowRight;
  } else if (settings().navigation_keys == 2 && vim_key) {
    mapped = key == 'k' ? Event::ArrowUp
             : key == 'j' ? Event::ArrowDown
             : key == 'h' ? Event::ArrowLeft
                          : Event::ArrowRight;
  } else {
    return true;
  }
  target->OnEvent(mapped);
  return true;
}

inline std::string combat_log_message(const std::string &message) {
  if (settings().detailed_combat_log) {
    return message;
  }
  const std::size_t line_end = message.find('\n');
  const std::string first_line = message.substr(0, line_end);
  const std::size_t sentence_end = first_line.find(". ");
  return first_line.substr(0, sentence_end == std::string::npos
                                  ? std::string::npos
                                  : sentence_end + 1);
}

inline Element page(std::string title = "", Element content = emptyElement(),
                    std::string footer = "",
                    Color accent = accent_color()) {
  Elements sections{text("RPG ENHANCED") | bold | color(accent)};
  if (!title.empty()) {
    sections.push_back(text(std::move(title)) | bold | color(accent));
  }
  sections.push_back(separator() | color(accent));
  sections.push_back(std::move(content));
  if (!footer.empty()) {
    sections.push_back(separator() | color(accent));
    sections.push_back(text(std::move(footer)) | dim);
  }
  Element result = vbox(std::move(sections)) | color(text_color());
  result = result | borderStyled(ROUNDED, accent);
  if (settings().reduced_motion) {
    result = result | size(WIDTH, EQUAL, 80);
    result = result | size(HEIGHT, EQUAL, 22);
  } else {
    result = result | size(WIDTH, LESS_THAN, 92);
  }
  return result | center;
}

inline Element panel(std::string title = "", Element content = emptyElement(),
                     Color accent = accent_color()) {
  if (title.empty()) {
    return std::move(content) | borderRounded | flex;
  }
  return window(text(std::move(title)) | bold | color(accent),
                std::move(content), ROUNDED) |
         flex;
}

inline Element meter(std::string label, std::uint64_t value,
                     std::uint64_t maximum, Color accent) {
  const float ratio = maximum == 0
                          ? 0.0F
                          : static_cast<float>(value) /
                                static_cast<float>(maximum);
  return vbox(
      {text(std::move(label)) | color(text_color()),
       hbox({gauge(std::clamp(ratio, 0.0F, 1.0F)) | color(accent) | flex,
             text(" " + std::to_string(value) + "/" + std::to_string(maximum)) |
                 color(text_color())})});
}

inline int choose_option(const std::string &title = "",
                         const std::vector<std::string> &options = {},
                         Element context = emptyElement(),
                         int reduced_motion_menu_height = 12) {
  if (options.empty()) {
    return -1;
  }

  auto screen = ScreenInteractive::Fullscreen();
  int selected = 0;
  auto menu_options = settings().reduced_motion ? MenuOption::Vertical()
                                                : MenuOption::VerticalAnimated();
  menu_options.on_enter = screen.ExitLoopClosure();
  auto menu = Menu(&options, &selected, menu_options);
  auto navigation = CatchEvent(menu, [&](Event event) {
    return handle_navigation_key(event, menu);
  });
  screen.Loop(Renderer(navigation, [&] {
    Element menu_element = menu->Render() | borderRounded |
                           color(accent_color());
    if (settings().reduced_motion) {
      menu_element = menu_element |
                     size(HEIGHT, EQUAL, reduced_motion_menu_height);
    }
    return page(title, vbox({context, std::move(menu_element)}), "");
  }));
  return selected;
}

inline void notify(const std::string &title = "",
                   const std::string &message = "",
                   Color accent = accent_color()) {
  auto screen = ScreenInteractive::Fullscreen();
  auto button_options = settings().reduced_motion
                            ? ButtonOption::Simple()
                            : ButtonOption::Animated(accent_color());
  auto continue_button = Button("Continue", screen.ExitLoopClosure(),
                                button_options);
  auto renderer = Renderer(continue_button, [&] {
    Element message_element = paragraph(message) | color(text_color());
    if (settings().reduced_motion) {
      message_element = message_element | size(HEIGHT, EQUAL, 8);
    }
    return page(title,
                vbox({std::move(message_element), continue_button->Render()}),
                "", accent);
  });
  screen.Loop(renderer);
}

inline std::string input_text(const std::string &title = "",
                              const std::string &prompt = "") {
  auto screen = ScreenInteractive::Fullscreen();
  std::string value;
  auto input_options = InputOption::Default();
  input_options.multiline = false;
  input_options.on_enter = screen.ExitLoopClosure();
  auto input = Input(&value, input_options);
  auto renderer = Renderer(input, [&] {
    return page(title,
                vbox({text(prompt) | color(text_color()),
                      input->Render() |
                          borderStyled(ROUNDED, accent_color())}),
                "");
  });
  screen.Loop(renderer);
  return value;
}

inline void settings_menu() {
  const auto choose_color = [](const std::string &title,
                               const std::vector<std::pair<std::string, Color>> &colors,
                               int &selection) {
    std::vector<std::string> options{"Use palette default"};
    for (const auto &option : colors) {
      options.push_back(option.first);
    }
    options.push_back("Back");
    const int chosen = choose_option(title, options,
                                     text("Sample text") | color(text_color()));
    if (chosen == 0) {
      selection = -1;
    } else if (chosen > 0 && chosen <= static_cast<int>(colors.size())) {
      selection = chosen - 1;
    }
  };

  while (true) {
    const std::vector<std::string> options{
        "Color palette: " + color_palette().name,
        "Text color: " + (settings().text_color < 0
                               ? std::string("Palette default")
                               : text_colors()[static_cast<std::size_t>(settings().text_color)].first),
        "Accent color: " + (settings().accent_color < 0
                                 ? std::string("Palette default")
                                 : accent_colors()[static_cast<std::size_t>(settings().accent_color)].first),
        "Reduced motion: " + std::string(settings().reduced_motion ? "On" : "Off"),
        "UI density: " + std::string(settings().compact_ui ? "Compact" : "Expanded"),
        "Combat log: " + std::string(settings().detailed_combat_log ? "Detailed" : "Concise"),
        "Navigation keys: " + std::vector<std::string>{"Arrows", "Arrows + WASD", "Arrows + Vim (HJKL)"}[
                                 static_cast<std::size_t>(settings().navigation_keys)],
        "Back",
    };
    const int selected = choose_option(
        "Settings", options,
        vbox({text("Text preview") | color(text_color()),
              text("Accent preview") | bold | color(accent_color())}));
    if (selected == 0) {
      std::vector<std::string> palette_names;
      for (const ColorPalette &palette : color_palettes()) {
        palette_names.push_back(palette.name);
      }
      const int chosen = choose_option("Color palette", palette_names);
      if (chosen >= 0 && chosen < static_cast<int>(palette_names.size())) {
        settings().palette = chosen;
        settings().text_color = -1;
        settings().accent_color = -1;
      }
    } else if (selected == 1) {
      choose_color("Text color", text_colors(), settings().text_color);
    } else if (selected == 2) {
      choose_color("Accent color", accent_colors(), settings().accent_color);
    } else if (selected == 3) {
      settings().reduced_motion = !settings().reduced_motion;
    } else if (selected == 4) {
      settings().compact_ui = !settings().compact_ui;
    } else if (selected == 5) {
      settings().detailed_combat_log = !settings().detailed_combat_log;
    } else if (selected == 6) {
      const int chosen = choose_option(
          "Navigation keys", {"Arrows", "WASD shortcuts", "Vim shortcuts"});
      if (chosen >= 0 && chosen <= 2) {
        settings().navigation_keys = chosen;
      }
    } else {
      return;
    }
    save_settings();
  }
}

} // namespace ui
} // namespace rpg
