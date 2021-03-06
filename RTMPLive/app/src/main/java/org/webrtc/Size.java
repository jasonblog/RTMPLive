package org.webrtc;
public class Size
{
    public int width;
    public int height;
    public Size(int width, int height)
    {
        this.width  = width;
        this.height = height;
    }

    @Override
    public String toString()
    {
        return width + "x" + height;
    }

    @Override
    public boolean equals(Object other)
    {
        if (!(other instanceof Size)) {
            return false;
        }

        final Size otherSize = (Size) other;
        return width == otherSize.width && height == otherSize.height;
    }

    @Override
    public int hashCode()
    {
        return 1 + 65537 * width + height;
    }
}
