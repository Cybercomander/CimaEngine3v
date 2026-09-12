#include "ICTimer.hpp"

namespace IVJ
{
// Todo timer arranca sin frames contados
ICTimer::ICTimer(int max_frame) : CE::IComponentes{}, curr_frame{0}, max_frame{max_frame}
{
}
} // namespace IVJ
