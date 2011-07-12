
#include "../TextureItem/TextureItem.h"

class AnimatedTexture
{
private:
	vector<TextureItem*> m_Frames;

protected:
public:
	AnimatedTexture();
	~AnimatedTexture() {}
};