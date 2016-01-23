#pragma once

#include <globjects/base/ref_ptr.h>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/InputSlot.h>


namespace globjects
{
    class Framebuffer;
    class Texture;
}

namespace gloperate
{
    class AbstractViewportCapability;
    class ScreenAlignedQuad;
}

class PostProcessingStage : public gloperate::AbstractStage
{
public:
    PostProcessingStage();

    virtual void initialize() override;

    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
    gloperate::InputSlot<int> currentFrame;

    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> color;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> depth;
    gloperate::InputSlot<globjects::ref_ptr<globjects::Texture>> normal;


protected:
    virtual void process() override;

    void resizeTexture(int width, int height);

    globjects::ref_ptr<globjects::Framebuffer> m_fbo;
    globjects::ref_ptr<globjects::Texture> m_accumulation;
    globjects::ref_ptr<gloperate::ScreenAlignedQuad> m_screenAlignedQuad;
};
