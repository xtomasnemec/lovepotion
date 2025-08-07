#include "common/Optional.hpp"

#include "modules/font/GenericShaper.hpp"
#include "modules/font/Rasterizer.hpp"

namespace love
{
    GenericShaper::GenericShaper(Rasterizer* rasterizer) : TextShaper(rasterizer)
    {}

    void GenericShaper::computeGlyphPositions(const ColoredCodepoints& codepoints, Range range,
                                              Vector2 offset, float extraspacing,
                                              std::vector<GlyphPosition>* positions,
                                              std::vector<IndexedColor>* colors, TextInfo* info)
    {
        if (!range.isValid())
            range = Range(0, codepoints.codepoints.size());

        const auto dataType = this->rasterizers[0]->getDataType();

        switch (dataType)
        {
            default:
            case Rasterizer::DATA_TRUETYPE:
                offset.y += this->getBaseline();
                break;
            case Rasterizer::DATA_BCFNT:
                offset.y += -this->getBaseline();
                break;
        }

        // Spacing counter and newline handling.
        Vector2 curpos         = offset;
        float spacingRemainder = 0.0f;

        int maxwidth       = 0;
        uint32_t prevglyph = 0;

        if (positions)
            positions->reserve(range.getSize());

        int colorindex = 0;
        int ncolors    = (int)codepoints.colors.size();
        Optional<Color> colorToAdd;

        // Make sure the right color is applied to the start of the glyph list,
        // when the start isn't 0.
        if (colors && range.getOffset() > 0 && !codepoints.colors.empty())
        {
            for (; colorindex < ncolors; colorindex++)
            {
                if (codepoints.colors[colorindex].index >= (int)range.getOffset())
                    break;

                colorToAdd.set(codepoints.colors[colorindex].color);
            }
        }

        for (int i = (int)range.getMin(); i <= (int)range.getMax(); i++)
        {
            uint32_t g = codepoints.codepoints[i];

            // Do this before anything else so we don't miss colors corresponding
            // to newlines. The actual add to the list happens after newline
            // handling, to make sure the resulting index is valid in the positions
            // array.
            if (colors && colorindex < ncolors && codepoints.colors[colorindex].index == i)
            {
                colorToAdd.set(codepoints.colors[colorindex].color);
                colorindex++;
            }

            if (g == '\n')
            {
                if (curpos.x > maxwidth)
                    maxwidth = (int)curpos.x;

                // Wrap newline, but do not output a position for it.
                curpos.y += floorf(getHeight() * getLineHeight() + 0.5f);
                curpos.x  = offset.x;
                prevglyph = 0;
                continue;
            }

            // Ignore carriage returns
            if (g == '\r')
            {
                prevglyph = g;
                continue;
            }

            if (colorToAdd.hasValue && colors && positions)
            {
                IndexedColor c = { colorToAdd.value, (int)positions->size() };
                colors->push_back(c);
                colorToAdd.clear();
            }

            // Add kerning to the current horizontal offset.
            curpos.x += getKerning(prevglyph, g);

            GlyphIndex glyphindex;
            int advance = getGlyphAdvance(g, &glyphindex);

            if (positions)
                positions->push_back({ Vector2(curpos.x, curpos.y), glyphindex });

            // Advance the x position for the next glyph.
            curpos.x += advance;

            // Account for extra spacing given to space characters.
            if (g == ' ' && extraspacing != 0.0f)
            {
                spacingRemainder += fmod(extraspacing, 1);
                curpos.x += floorf(extraspacing) + floorf(spacingRemainder);
                spacingRemainder = fmod(spacingRemainder, 1);
            }

            prevglyph = g;
        }

        if (curpos.x > maxwidth)
            maxwidth = (int)curpos.x;

        if (info != nullptr)
        {
            info->width  = maxwidth - offset.x;
            info->height = curpos.y - offset.y;
            if (curpos.x > offset.x)
                info->height += floorf(getHeight() * getLineHeight() + 0.5f);
        }
    }

    int GenericShaper::computeWordWrapIndex(const ColoredCodepoints& codepoints, Range range, float wraplimit,
                                            float* width)
    {
        if (!range.isValid())
            range = Range(0, codepoints.codepoints.size());

        uint32_t prevglyph = 0;

        float w                    = 0.0f;
        float outwidth             = 0.0f;
        float widthbeforelastspace = 0.0f;
        int firstindexafterspace   = -1;

        for (int i = (int)range.getMin(); i <= (int)range.getMax(); i++)
        {
            uint32_t g = codepoints.codepoints[i];

            if (g == '\r')
            {
                prevglyph = g;
                continue;
            }

            float newwidth = w + getKerning(prevglyph, g) + getGlyphAdvance(g);

            // Don't count trailing spaces in the output width.
            if (isWhitespace(g))
            {
                if (!isWhitespace(prevglyph))
                    widthbeforelastspace = w;
            }
            else
            {
                if (isWhitespace(prevglyph))
                    firstindexafterspace = i;

                // Only wrap when there's a non-space character.
                if (newwidth > wraplimit)
                {
                    // If this is the first character, wrap from the next one instead of this one.
                    int wrapindex = i > (int)range.first ? i : (int)range.first + 1;

                    // Rewind to after the last seen space when wrapping.
                    if (firstindexafterspace != -1)
                    {
                        wrapindex = firstindexafterspace;
                        outwidth  = widthbeforelastspace;
                    }

                    if (width)
                        *width = outwidth;

                    return wrapindex;
                }

                outwidth = newwidth;
            }

            w         = newwidth;
            prevglyph = g;
        }

        if (width)
            *width = outwidth;

        // There wasn't any wrap in the middle of the range.
        return range.last + 1;
    }
} // namespace love
