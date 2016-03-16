#include "MultiFramePipeline.h"

#include "ModelLoadingStage.h"
#include "RasterizationStage.h"
#include "PostprocessingStage.h"
#include "FrameAccumulationStage.h"
#include "BlitStage.h"


MultiFramePipeline::MultiFramePipeline()
: gloperate::AbstractPipeline("MultiframeSampling")
, resourceManager(nullptr)
, multiFrameCount(64)
, preset(Preset::Jakobi)
, useReflections(false)
{
    auto modelLoadingStage = new ModelLoadingStage();
    auto rasterizationStage = new RasterizationStage();
    auto postprocessingStage = new PostprocessingStage();
    auto frameAccumulationStage = new FrameAccumulationStage();
    auto blitStage = new BlitStage();

    modelLoadingStage->resourceManager = resourceManager;
    modelLoadingStage->preset = preset;

    rasterizationStage->projection = projection;
    rasterizationStage->camera = camera;
    rasterizationStage->viewport = viewport;
    rasterizationStage->multiFrameCount = multiFrameCount;
    rasterizationStage->useReflections = useReflections;
    rasterizationStage->useDOF = useDOF;
    rasterizationStage->drawablesMap = modelLoadingStage->drawablesMap;
    rasterizationStage->materialMap = modelLoadingStage->materialMap;
    rasterizationStage->presetInformation = modelLoadingStage->presetInformation;

    postprocessingStage->viewport = viewport;
    postprocessingStage->camera = camera;
    postprocessingStage->projection = projection;
    postprocessingStage->presetInformation = modelLoadingStage->presetInformation;
    postprocessingStage->useReflections = useReflections;
    postprocessingStage->color = rasterizationStage->color;
    postprocessingStage->normal = rasterizationStage->normal;
    postprocessingStage->depth = rasterizationStage->depth;
    postprocessingStage->worldPos = rasterizationStage->worldPos;
    postprocessingStage->reflectMask = rasterizationStage->reflectMask;

    frameAccumulationStage->viewport = viewport;
    frameAccumulationStage->currentFrame = rasterizationStage->currentFrame;
    frameAccumulationStage->frame = postprocessingStage->postprocessedFrame;
    frameAccumulationStage->depth = rasterizationStage->depth;

    blitStage->viewport = viewport;
    blitStage->accumulation = frameAccumulationStage->accumulation;
    blitStage->depth = rasterizationStage->depth;

    addStages(modelLoadingStage, rasterizationStage, postprocessingStage, frameAccumulationStage, blitStage);
}
