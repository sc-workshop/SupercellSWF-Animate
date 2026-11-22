#pragma once

#include "AnimatePublisher.h"
#include "flash/flash.h"

namespace sc::Adobe {
    class SCWriter;

    namespace Cache {
        class SymbolCache;
    }

    class SCMovieclipWriter : public Animate::Publisher::SharedMovieclipWriter {
    private:
        SCWriter& m_writer;
        uint32_t m_frameCount = 0;

        // Current object
        flash::MovieClip m_object;

        // Bank with current object transformations only
        flash::MatrixBank m_bank;

        // Helper functions

        uint16_t GetInstanceIndex(uint16_t elementsCount, uint16_t id, uint8_t blendIndex, std::string name);

        void FinalizeTransforms();

    public:
        SCMovieclipWriter(SCWriter& writer, Animate::Publisher::SymbolContext& symbol);
        virtual ~SCMovieclipWriter();
        virtual void Next();
        virtual void InitializeTimeline(uint32_t fps, uint32_t frameCount);
        virtual void SetLabel(const std::u16string& label);
        virtual void AddFrameElement(Animate::Publisher::ResourceReference resource,
                                     const Animate::Publisher::DisplayObjectProperties& properties,
                                     const std::u16string& name,
                                     std::optional<Animate::DOM::Utils::MATRIX2D> matrix,
                                     std::optional<Animate::DOM::Utils::COLOR_MATRIX> color);

        virtual bool Finalize(Animate::Publisher::ResourceReference reference, bool required, bool new_symbol);

        virtual bool HasContent() const override;

        virtual void ApplyResourceRemap(const std::unordered_map<uint16_t, uint16_t>& remap,
                                        bool rebuild_instances = true) override;

    protected:
        virtual std::size_t GenerateHash() const;

    private:
        friend class Cache::SymbolCache;
    };
}