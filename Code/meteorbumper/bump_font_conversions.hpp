#pragma once

namespace bump
{
	
	namespace font
	{
		
		inline double points_to_pixels(double point_size, double resolution_dpi)
		{
			return (point_size / 72.0) * resolution_dpi; // 72 points per inch
		}

		inline double pixels_to_points(double pixel_size, double resolution_dpi)
		{
			return (pixel_size / resolution_dpi) * 72.0;
		}

		inline double font_units_to_pixels(double font_units, double font_units_per_em, double pixels_per_em)
		{
			return (font_units / font_units_per_em) * pixels_per_em;
		}

		inline double font_units_to_pixels(double font_units, double font_units_per_em, double points_per_em, double resolution_dpi)
		{
			return font_units_to_pixels(font_units, font_units_per_em, points_to_pixels(points_per_em, resolution_dpi));
		}
		
	} // font
	
} // bump
