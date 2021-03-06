#ifndef Mapping_h
#define Mapping_h

#include <algorithm>      // for transform
#include <fstream>        // for size_t
#include <functional>     // for function
#include <iterator>       // for back_insert_iterator, back_inserter
#include <optional>       // for optional, nullopt
#include <set>            // for set
#include <stdexcept>      // for out_of_range
#include <string>         // for string, basic_string, allocator, operator+
#include <unordered_map>  // for unordered_map, unordered_map<>::mapped_type
#include <utility>        // for move, pair, make_pair
#include <vector>         // for vector, vector<>::iterator
#include "config.hpp"     // for Config
#include "midi.hpp"       // for Midi
#include "utils.hpp"      // for bindFirst, constructor

class Mappings {
  private:
    struct ControlOutput {
      const std::string output;
      const std::string path;
      bool inverted = false;
      ControlOutput() = default;
      ControlOutput(std::string output, std::string path, bool inverted) : output(std::move(output)), path(std::move(path)), inverted(inverted) {}
      static ControlOutput parse(const std::string& str, std::size_t start = 0);
      std::string encode() const { return output + ":" + path + ":" + (inverted ? "true" : "false"); }
    };
    struct ChannelOutput {
      const std::string output;
      const std::string channel;
      ChannelOutput() = default;
      ChannelOutput(std::string output, std::string channel) : output(std::move(output)), channel(std::move(channel)) {}
      static ChannelOutput parse(const std::string& str, std::size_t start = 0);
      std::string encode() const { return output + ":" + channel; }
    };
    struct ActionOutput {
      const std::string action;
      ActionOutput() = default;
      explicit ActionOutput(std::string action) : action(std::move(action)) {}
      static ActionOutput parse(const std::string& str, std::size_t start = 0);
      std::string encode() const { return action; }
    };
    struct Mapping {
      private:
        const Config& config;
      public:
        const std::string filename;
      private:
        std::unordered_map<std::string, ControlOutput> controls;
        std::unordered_map<std::string, ChannelOutput> channels;
        std::unordered_map<std::string, ActionOutput> actions;
        std::unordered_map<std::string, std::pair<std::string, bool>> feedbacks;
      public:
        Mapping(const Config& config, const std::string& filename);
        Mapping() = delete;

        void save() const;

        std::optional<ControlOutput> outputFromString(const std::string& str);

        std::string encodedMappingOf(const std::string& controlName);

        std::optional<std::pair<std::string, bool>> feedbackFor(const std::string& str) {
          try {
            return feedbacks.at(str);
          } catch (std::out_of_range&) {
            return std::nullopt;
          }
        }

        void addControl(const std::string& control, ControlOutput&& output) {
          controls.erase(control);
          controls.emplace(control, output);
        }
        void removeControl(const std::string& control) {
          controls.erase(control);
        }
        void addChannel(const std::string& channel, ChannelOutput&& output) {
          channels.erase(channel);
          channels.emplace(channel, output);
        }
        void removeChannel(const std::string& channel) {
          channels.erase(channel);
        }
        void addAction(const std::string& action, ActionOutput&& output) {
          actions.erase(action);
          actions.emplace(action, output);
        }
        void removeAction(const std::string& action) {
          actions.erase(action);
        }
        void addFeedback(const std::string& control) {
          std::optional<ControlOutput> output = outputFromString(control);
          if (output) {
            feedbacks[output->path] = std::make_pair(control, output->inverted);
          }
        }
      private:
        template <typename T>
          void addFeedback(T controls) {
            for (const std::string& control : controls) {
              addFeedback(control);
            }
          }
      public:
        void addFeedbackForChannel(const std::string& channel) {
          addFeedback(config.controlsForChannel(channel));
        }
        void addFeedbackForAction(const std::string& action) {
          addFeedback(config.controlsForAction(action));
        }
    };
    Config& config;
    std::vector<Mapping> mappings;
    std::size_t currentMappingIndex = 0;
    Mapping& currentMapping() { return mappings[currentMappingIndex]; }

    void refreshBank();

    void midiCallback(const Midi::Event& event);
    void midiMockCallback(const std::string& led, bool value);
    void outputCallback(const std::string& path, float v);
    void guiOpenCallback();
    void guiRecvCallback(const std::string& str);

    void load() {
      std::transform(config.banks.begin(), config.banks.end(), std::back_inserter(mappings), bindFirst<Mapping, const Config&, std::string>(Constructor<Mapping>(), this->config));
    }
  public:
    Mappings(Config& config);
    void save() {
      for (const Mapping& mapping : mappings) {
        mapping.save();
      }
    }
};

#endif

