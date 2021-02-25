

/**************************************************************************************************
 *** This file was autogenerated from GrChildProcessorInlineFieldAccess.fp; do not modify.
 **************************************************************************************************/
#ifndef GrChildProcessorInlineFieldAccess_DEFINED
#define GrChildProcessorInlineFieldAccess_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"


#include "src/gpu/GrFragmentProcessor.h"

class GrChildProcessorInlineFieldAccess : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child) {
        return std::unique_ptr<GrFragmentProcessor>(new GrChildProcessorInlineFieldAccess(std::move(child)));
    }
    GrChildProcessorInlineFieldAccess(const GrChildProcessorInlineFieldAccess& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "ChildProcessorInlineFieldAccess"; }
private:
    GrChildProcessorInlineFieldAccess(std::unique_ptr<GrFragmentProcessor> child)
    : INHERITED(kGrChildProcessorInlineFieldAccess_ClassID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child), SkSL::SampleUsage::PassThrough());    }
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    using INHERITED = GrFragmentProcessor;
};
#endif
