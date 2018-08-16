/**
* @file TweenAnimation.cpp
*/
#include "TweenAnimation.h"
#include "Node.h"
#include <algorithm>
#include <iostream>

namespace TweenAnimation {

/**
* �R���X�g���N�^.
*
* @param d  ���쎞��.
* @param e  ��ԕ��@.
* @param t  ���[�v��.
*/
Tween::Tween(glm::f32 d, EasingType e, glm::u32 t) :
  duration(d),
  reciprocalDuration(1.0f / d),
  times(static_cast<float>(t)),
  easing(e)
{
}

/**
* ��Ԃ��X�V����.
*
* @param node    �X�V�Ώۂ̃m�[�h.
* @param elapsed �o�ߎ���.
*/
void Tween::UpdateWithEasing(Node& node, glm::f32 elapsed)
{
  const glm::u32 current = static_cast<glm::u32>(elapsed * ReciprocalUnitDuration());
  for (glm::u32 i = total; i < current; ++i) {
    Update(node, UnitDuration());
    Initialize(node);
  }
  total = current;
  glm::f32 ratio = std::fmod(elapsed, UnitDuration()) * ReciprocalUnitDuration();
  switch (easing) {
  default:
  case EasingType::Linear:
    /* �������Ȃ� */
    break;
  case EasingType::EaseIn:
    ratio *= ratio;
    break;
  case EasingType::EaseOut:
    ratio = 2.0f * ratio - ratio * ratio;
    break;
  case EasingType::EaseInOut:
    ratio *= 2.0f;
    if (ratio < 1.0f) {
      ratio *= ratio;
    } else {
      ratio -= 1.0f;
      ratio = 2.0f * ratio - ratio * ratio;
      ratio += 1.0f;
    }
    ratio *= 0.5f;
    break;
  case EasingType::EaseOutBack: {
    static const float scale = 1.70158f * 1.525f;
    ratio -= 1;
    ratio = 1 + 2.70158f * ratio * ratio * ratio + 1.70158f * ratio * ratio;
    break;
  }
  case EasingType::EaseOutBounce:
    if (ratio < (1 / 2.75f)) {
      ratio = 7.5625f * ratio * ratio;
    } else if (ratio < (2 / 2.75f)) {
      ratio -= 1.5f / 2.75f;
      ratio = 7.5625f * ratio * ratio + 0.75f;
    } else if (ratio < (2.5 / 2.75f)) {
      ratio -= 2.25f / 2.75f;
      ratio = 7.5625f * ratio * ratio + 0.9375f;
    } else {
      ratio -= 2.625f / 2.75f;
      ratio = 7.5625f * ratio * ratio + 0.984375f;
    }
    break;
  }
  Update(node, UnitDuration() * ratio);
}

/**
* ���䂷��g�E�B�[�j���O�I�u�W�F�N�g��ݒ肷��.
*
* @param p �g�E�B�[�j���O�I�u�W�F�N�g�ւ̃|�C���^.
*/
void Animate::Tween(const TweenPtr& p)
{
  tween = p;
  elapsed = 0.0f;
  isInitialized = false;
}

/**
* ��Ԃ��X�V����.
*
* @param node  �X�V�Ώۂ̃m�[�h.
* @param dt    �O��̍X�V����̌o�ߎ���.
*/
void Animate::Update(Node& node, glm::f32 dt)
{
  if (!tween) {
    return;
  }
  if (!isInitialized) {
    isInitialized = true;
    tween->Initialize(node);
  }
  elapsed += dt;
  if (elapsed >= tween->TotalDuration() && isLoop) {
    tween->UpdateWithEasing(node, tween->TotalDuration());
    tween->Initialize(node);
    elapsed -= tween->TotalDuration();
  }
  tween->UpdateWithEasing(node, elapsed);
}

/**
* �R���X�g���N�^.
*
* @param d  ���쎞��.
* @param v  �ړ����鋗��.
*/
MoveBy::MoveBy(glm::f32 d, const glm::vec3& v, EasingType e, Target t)
  : Tween(d, e)
  , vector(v)
  , target(t)
{
}

/**
* �ړ���Ԃ�����������.
*
* @param node ����Ώۂ̃m�[�h.
*/
void MoveBy::Initialize(Node& node)
{
  Tween::Initialize(node);
  start = node.Position();
}

/**
* �ړ���Ԃ��X�V����.
*
* @param node    �X�V�Ώۂ̃m�[�h.
* @param elapsed �o�ߎ���.
*/
void MoveBy::Update(Node& node, glm::f32 elapsed)
{
  const glm::f32 ratio = elapsed * ReciprocalUnitDuration();
  const glm::vec3 cur = start + vector * ratio;
  glm::vec3 pos = node.Position();
  if (static_cast<int>(target) & static_cast<int>(Target::X)) {
    pos.x = cur.x;
  }
  if (static_cast<int>(target) & static_cast<int>(Target::Y)) {
    pos.y = cur.y;
  }
  if (static_cast<int>(target) & static_cast<int>(Target::Z)) {
    pos.z = cur.z;
  }
  node.Position(pos);
}

/**
* �g�E�B�[�j���O�I�u�W�F�N�g��ǉ�����.
*
* @param p �ǉ�����g�E�B�[�j���O�I�u�W�F�N�g.
*/
void Sequence::Add(const TweenPtr& p)
{
  seq.push_back(p);
  UnitDuration(UnitDuration() + p->TotalDuration());
}

/**
* ���̃g�E�B�[�j���O�I�u�W�F�N�g��ݒ肷��.
*
* @param node ����Ώۂ̃m�[�h.
*/
bool Sequence::NextTween(Node& node)
{
  if (index >= static_cast<int>(seq.size()) - 1) {
    return false;
  }
  ++index;
  currentDurationBegin = currentDurationEnd;
  currentDurationEnd += seq[index]->TotalDuration();
  seq[index]->Initialize(node);
  return true;
}

/**
* �g�E�B�[�j���O�������������.
*
* @param node ����Ώۂ̃m�[�h.
*/
void Sequence::Initialize(Node& node)
{
  Tween::Initialize(node);
  index = -1;
  currentDurationEnd = 0.0f;
  NextTween(node);
}

/**
* �g�E�B�[�j���O����X�V����.
*
* @param node    �X�V�Ώۂ̃m�[�h.
* @param elapsed �o�ߎ���.
*/
void Sequence::Update(Node& node, glm::f32 elapsed)
{
  if (seq.empty()) {
    return;
  }
  while (elapsed >= currentDurationEnd) {
    seq[index]->UpdateWithEasing(node, seq[index]->TotalDuration());
    if (!NextTween(node)) {
      return;
    }
  }
  seq[index]->UpdateWithEasing(node, elapsed - currentDurationBegin);
}

/**
* �ړ���Ԃ�����������.
*
* @param node ����Ώۂ̃m�[�h.
*/
void Parallelize::Initialize(Node& node)
{
  Tween::Initialize(node);
  for (auto& e : tweens) {
    e->Initialize(node);
  }
}

/**
* �ړ���Ԃ��X�V����.
*
* @param node    �X�V�Ώۂ̃m�[�h.
* @param elapsed �o�ߎ���.
*/
void Parallelize::Update(Node& node, glm::f32 elapsed)
{
  if (tweens.empty()) {
    return;
  }
  for (auto& e : tweens) {
    e->UpdateWithEasing(node, std::min(e->TotalDuration(), elapsed));
  }
}

/**
* ����Ɏ��s����g�E�B�[�j���O�I�u�W�F�N�g��ǉ�����.
*
* @param p �ǉ�����g�E�B�[�j���O�I�u�W�F�N�g.
*/
void Parallelize::Add(const TweenPtr& p)
{
  tweens.push_back(p);
  const glm::f32 d = p->TotalDuration();
  if (d > UnitDuration()) {
    UnitDuration(d);
  }
}

void RemoveFromParent::Update(Node& node, glm::f32 elapsed)
{
  if (node.Parent()) {
    node.Parent()->RemoveChild(&node);
  }
}

/**
* ��]�A�j���[�V����������������.
*
* @param node ����Ώۂ̃m�[�h.
*/
void Rotation::Initialize(Node& node)
{
  start = node.Rotation();
}

/**
* ��]�A�j���[�V�������X�V����.
*
* @param node    �X�V�Ώۂ̃m�[�h.
* @param elapsed �o�ߎ���.
*/
void Rotation::Update(Node& node, glm::f32 dt)
{
  const glm::f32 ratio = dt * ReciprocalUnitDuration();
  node.Rotation(start + rotation * ratio);
}

/**
* �R���X�g���N�^.
*
* @param d  ���쎞��.
* @param v  �g��E�k����.
* @param e  ��ԕ��@.
* @param t  ����ΏۂƂ��鎲.
*/
Scaling::Scaling(glm::f32 d, const glm::vec2& v, EasingType e, Target t)
  : Tween(d, e)
  , vector(v)
  , target(t)
{
}

/**
* �g��E�k���A�j���[�V����������������.
*
* @param node ����Ώۂ̃m�[�h.
*/
void Scaling::Initialize(Node& node)
{
  Tween::Initialize(node);
  start = node.Position();
}

/**
* �g��E�k���A�j���[�V�������X�V����.
*
* @param node    �X�V�Ώۂ̃m�[�h.
* @param elapsed �o�ߎ���.
*/
void Scaling::Update(Node& node, glm::f32 elapsed)
{
  const glm::f32 ratio = elapsed * ReciprocalUnitDuration();
  const glm::vec2 cur = start + vector * ratio;
  glm::vec2 tmp = node.Scale();
  if (static_cast<int>(target) & static_cast<int>(Target::X)) {
    tmp.x = cur.x;
  }
  if (static_cast<int>(target) & static_cast<int>(Target::Y)) {
    tmp.y = cur.y;
  }
  node.Scale(tmp);
}

/**
* �R���X�g���N�^.
*/
RemoveIfOutOfArea::RemoveIfOutOfArea(const glm::vec2& origin, const glm::vec2& size) :
  Tween(100, TweenAnimation::EasingType::Linear), origin(origin), size(size)
{
}

/**
* �X�V.
*
* @param node    �X�V�Ώۂ̃m�[�h.
* @param elapsed �o�ߎ���.
*/
void RemoveIfOutOfArea::Update(Node& node, glm::f32 dt)
{
  const glm::vec3 pos = node.Position();
  if (pos.x < origin.x || pos.x > origin.x + size.x ||
    pos.y < origin.y || pos.y > origin.y + size.y) {
    if (node.Parent()) {
      node.Parent()->RemoveChild(&node);
    }
  }
}

} // namespace TweenAnimation