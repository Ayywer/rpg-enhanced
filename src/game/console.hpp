#pragma once

#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace rpg {
namespace ui {

using namespace ftxui;
inline Element page(std::string title = "", Element content = emptyElement(),
                    std::string footer = "",
                    Color accent = Color::RGB(91, 205, 183)) {
  Elements sections{text("RPG ENHANCED") | bold | color(accent)};
  if (!title.empty()) {
    sections.push_back(text(std::move(title)) | bold | color(accent));
  }
  sections.push_back(separator() | color(Color::RGB(58, 86, 91)));
  sections.push_back(std::move(content));
  if (!footer.empty()) {
    sections.push_back(separator() | color(Color::RGB(58, 86, 91)));
    sections.push_back(text(std::move(footer)) | dim);
  }
  return vbox(std::move(sections)) |
         borderStyled(ROUNDED, Color::RGB(64, 113, 112)) |
         size(WIDTH, LESS_THAN, 92) | center;
}

inline Element panel(std::string title = "", Element content = emptyElement(),
                     Color accent = Color::RGB(91, 205, 183)) {
  if (title.empty()) {
    return std::move(content) | borderRounded | flex;
  }
  return window(text(std::move(title)) | bold | color(accent),
                std::move(content), ROUNDED) |
         flex;
}

inline Element meter(std::string label, std::uint64_t value,
                     std::uint64_t maximum, Color accent) {
  const float ratio =
      maximum == 0 ? 0.0F
                   : static_cast<float>(value) / static_cast<float>(maximum);
  return vbox(
      {text(std::move(label)) | color(Color::GrayLight),
       hbox({gauge(std::clamp(ratio, 0.0F, 1.0F)) | color(accent) | flex,
             text(" " + std::to_string(value) + "/" + std::to_string(maximum)) |
                 color(Color::White)})});
}

inline int choose_option(const std::string &title = "",
                         const std::vector<std::string> &options = {},
                         Element context = emptyElement()) {
  if (options.empty()) {
    return -1;
  }

  auto screen = ScreenInteractive::Fullscreen();
  int selected = 0;
  auto menu_options = MenuOption::VerticalAnimated();
  menu_options.on_enter = screen.ExitLoopClosure();
  auto menu = Menu(&options, &selected, menu_options);
  auto renderer = Renderer(menu, [&] {
    return page(title,
                vbox({context, menu->Render() | borderRounded |
                                   color(Color::RGB(64, 113, 112))}),
                "");
  });
  screen.Loop(renderer);
  return selected;
}

inline void notify(const std::string &title = "",
                   const std::string &message = "",
                   Color accent = Color::RGB(91, 205, 183)) {
  auto screen = ScreenInteractive::Fullscreen();
  auto continue_button = Button("Continue", screen.ExitLoopClosure(),
                                ButtonOption::Animated(Color::RGB(33, 93, 83)));
  auto renderer = Renderer(continue_button, [&] {
    return page(title,
                vbox(Elements{paragraph(message) | color(Color::White),
                              continue_button->Render()}),
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
                vbox({text(prompt) | color(Color::GrayLight),
                      input->Render() |
                          borderStyled(ROUNDED, Color::RGB(64, 113, 112))}),
          "");
  });
  screen.Loop(renderer);
  return value;
}

} // namespace ui

} // namespace rpg