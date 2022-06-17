#include "third-party/fmt/core.h"
#include "common/util/json_util.h"
#include "common/util/FileUtil.h"
#include "tools/build_level/LevelFile.h"
#include "tools/build_level/FileInfo.h"
#include "tools/build_level/Tfrag.h"

#include "common/custom_data/Tfrag3Data.h"
#include "common/util/compress.h"

void save_pc_data(const std::string& nickname, tfrag3::Level& data) {
  Serializer ser;
  data.serialize(ser);
  auto compressed =
      compression::compress_zstd(ser.get_save_result().first, ser.get_save_result().second);
  fmt::print("stats for {}\n", data.level_name);
  print_memory_usage(data, ser.get_save_result().second);
  fmt::print("compressed: {} -> {} ({:.2f}%)\n", ser.get_save_result().second, compressed.size(),
             100.f * compressed.size() / ser.get_save_result().second);
  file_util::write_binary_file(file_util::get_file_path({fmt::format("assets/{}.fr3", nickname)}),
                               compressed.data(), compressed.size());
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fmt::print("usage: buildlevel <level_description_json>\n");
    return 1;
  }

  fmt::print("buildlevel\n");
  file_util::setup_project_path({});

  std::string level_description_text = file_util::read_text_file(argv[1]);
  auto level_json = parse_commented_json(level_description_text, argv[1]);

  LevelFile file;          // GOAL level file
  tfrag3::Level pc_level;  // PC level file
  TexturePool tex_pool;    // pc level texture pool

  // add stuff
  file.info = make_file_info_for_level(std::filesystem::path(argv[1]).filename().string());
  // all vis
  // drawable trees
  // pat
  // texture remap
  // texture ids
  // unk zero
  // name
  file.name = level_json.at("long_name").get<std::string>();
  // nick
  file.nickname = level_json.at("nickname").get<std::string>();
  // vis infos
  // actors
  // cameras
  // nodes
  // boxes
  // ambients
  // subdivs
  // adgifs
  // actor birht
  // split box

  pc_level.level_name = file.name;

  // DRAWABLE TREE STUFF
  tfrag_from_gltf(file_util::get_file_path({level_json.at("tfrag_data").get<std::string>()}),
                  file.drawable_trees.tfrags.emplace_back(), pc_level.tfrag_trees[0].emplace_back(),
                  &tex_pool);

  auto result = file.save_object_file();
  fmt::print("Level bsp file size {} bytes\n", result.size());
  auto save_path = file_util::get_file_path({"buildlevel_out", fmt::format("{}.go", file.name)});
  file_util::create_dir_if_needed_for_file(save_path);
  fmt::print("Saving to {}\n", save_path);
  file_util::write_binary_file(save_path, result.data(), result.size());

  pc_level.textures = std::move(tex_pool.textures_by_idx);
  save_pc_data(file.nickname, pc_level);

  return 0;
}