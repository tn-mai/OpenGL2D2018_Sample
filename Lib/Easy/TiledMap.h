/**
* @file TiledMap.h
*/
#ifndef EASY_TILEDMAP_H_INCLUDED
#define EASY_TILEDMAP_H_INCLUDED
#include "Sprite.h"
#include <vector>
#include <cstdint>

/**
* 2D�^�C���}�b�v���Ǘ�����.
*
* Tiled Map Editor����G�N�X�|�[�g���ꂽJSON�t�@�C����ǂݍ��݁A�v���O�����ɂ���đ���\�ȃf�[�^���\�z����.
*
* - �ǂݍ���
*   TiledMap tm;
*   tm.Load("json_from_tme.json");
*
* - �l�̎擾.
*   tm.GetLayer(0)
*/
class TiledMap
{
public:
  /// TMX���C���[.
  struct Layer
  {
    uint32_t& At(int y, int x) { return mapData[y * size.x + x]; }
    uint32_t At(int y, int x) const { return mapData[y * size.x + x]; }

    std::unique_ptr<SpriteRenderer> renderer; ///< �}�b�v�`��I�u�W�F�N�g(�ύX�֎~).
    std::vector<uint32_t> mapData; ///< �}�b�v�f�[�^.
    glm::ivec2 size; ///< �}�b�v�̏c���̃f�[�^��(�ύX�֎~).
    glm::vec2 baseOffset; ///< �\���ʒu�̊�{�I�t�Z�b�g(�ύX�֎~).
    glm::vec2 offset; ///< �I�t�Z�b�g�␳�l.
    bool visible; ///< �\���E��\���𐧌䂷��t���O. true=�`�悳���. false=�`�悳��Ȃ�.
    bool drawable; ///< �`��f�[�^�̗L��(�ύX�֎~). true=�`��\. false=�`��s��.
    bool dirty; ///< �`��f�[�^�̍č\�z�𐧌䂷��t���O. true=�č\�z����. false=�\�z�ς݂܂��͕`��f�[�^�Ȃ�.
    int tilesetNo; ///< �Ή�����^�C���Z�b�g�̃C���f�b�N�X.
    float opacity; ///< �����x. 0.0(����)�`1.0(�s����).
  };

  /// TMX�^�C���Z�b�g.
  struct TileSet
  {
    glm::vec2 size; ///< �^�C���̏c���̃s�N�Z����.
    int columns; ///< ���̃^�C����.
    int firstId; ///< �ŏ���(�����)�^�C����ID.
    TexturePtr image; ///< �e�N�X�`��.
  };

  /// �^�C���̌����𐧌䂷��t���O.
  enum Flags {
    FlipH = 0x8000'0000u, ///< ���E���].
    FlipV = 0x4000'0000u, ///< �㉺���].
    Rot90 = 0x2000'0000u, ///< 90�x��].
    MaskForID = 0x1fff'ffffu, ///< �f�[�^����t���O�����O����ID���������o�����߂̃}�X�N�l.
  };

  TiledMap() = default;
  explicit TiledMap(const char* filename, uint32_t drawable = 0) { Load(filename, drawable); }
  TiledMap(const TiledMap&) = default;
  TiledMap& operator=(const TiledMap&) = default;
  ~TiledMap() = default;

  bool Load(const char* filename, uint32_t drawable = 0);
  void Unload();
  void Update();
  void Draw(const glm::vec2&) const;
  const glm::vec2& Position() const { return position; }
  void Position(const glm::vec2& pos) { position = pos; }
  size_t LayerCount() const { return layers.size(); }
  Layer& GetLayer(int n) { return layers[n]; }
  const Layer& GetLayer(int n) const { return layers[n]; }
  size_t TileSetCount() const { return tilesets.size(); }
  const TileSet& GetTileSet(int n) const { return tilesets[n]; }

private:
  std::vector<Layer> layers;
  std::vector<TileSet> tilesets;
  glm::vec2 position;
};


#endif // EASY_TILEDMAP_H_INCLUDED
