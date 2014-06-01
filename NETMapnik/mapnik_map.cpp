#include "stdafx.h"
#include "mapnik_map.h"

#include <mapnik\map.hpp>
#include <mapnik\load_map.hpp>
#include <mapnik\agg_renderer.hpp>
#include <mapnik\image_util.hpp>
#include <mapnik\grid\grid.hpp>
#include <mapnik\grid\grid_view.hpp>
#include <mapnik\grid\grid_renderer.hpp>
#include <mapnik\grid\grid_util.hpp>

#include <msclr\marshal_cppstd.h>

namespace NETMapnik
{
	//Constructor
	Map::Map()
	{
		_map = new mapnik::Map();
	}

	//Destructor
	Map::~Map()
	{
		if (_map != NULL)
			delete _map;
	}

	//width property
	System::UInt32 Map::Width::get()
	{
		return _map->width();
	}

	void Map::Width::set(System::UInt32 value)
	{
		_map->set_width(value);
	}

	//height property
	System::UInt32 Map::Height::get()
	{
		return _map->height();
	}

	void Map::Height::set(System::UInt32 value)
	{
		_map->set_height(value);
	}

	//load map
	void Map::LoadMap(System::String^ path)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		try
		{
			mapnik::load_map(*_map,unmanagedPath);
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
	}

	//zoom_to_box
	void Map::ZoomToBox(System::Double minx, System::Double miny, System::Double maxx, System::Double maxy)
	{
		_map->zoom_to_box(mapnik::box2d<double>(minx,miny,maxx,maxy));
	}

	//save to byte array
	array<System::Byte>^ Map::SaveToBytes(System::String^ format)
	{
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		mapnik::image_32 buf(_map->width(),_map->height());
		mapnik::agg_renderer<mapnik::image_32> ren(*_map,buf);
		try
		{
			ren.apply();
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
		std::string s = save_to_string(buf,unmanagedFormat);
		array<System::Byte>^ data = gcnew array<System::Byte>(s.size());
		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(&s[0]), data, 0, s.size());
		return data;
	}

	//save to file
	void Map::SaveToFile(System::String^ path, System::String^ format)
	{
		std::string unmanagedPath = msclr::interop::marshal_as<std::string>(path);
		std::string unmanagedFormat = msclr::interop::marshal_as<std::string>(format);

		mapnik::image_32 buf(_map->width(),_map->height());
		mapnik::agg_renderer<mapnik::image_32> ren(*_map,buf);
		try
		{
			ren.apply();
		}
		catch (const std::exception& ex)
		{
			System::String^ managedException = msclr::interop::marshal_as<System::String^>(ex.what());
			throw gcnew System::Exception(managedException);
		}
		mapnik::save_to_file(buf,unmanagedPath,unmanagedFormat);
	}

	void Map::RenderLayer(Grid^ grid, System::UInt32 layerIdx,  System::Collections::Generic::List<System::String^>^ fields)
	{
		mapnik::grid* g = grid->NativeObject();
		
		std::vector<mapnik::layer> const& layers = _map->layers();
		std::size_t layer_num = layers.size();
		if (layerIdx >= layer_num) {
			throw gcnew System::Exception(System::String::Format("Zero-based layer index {0} not valid only {1} layers in map",layerIdx,layer_num));
		}
		
		// convert .NET list to std::set
		for each(System::String^ name in fields)
		{
			g->add_property_name(msclr::interop::marshal_as<std::string>(name));
		}

		// copy property names
		std::set<std::string> attributes = g->property_names();
		// todo - make this a static constant
		std::string known_id_key = "__id__";
		if (attributes.find(known_id_key) != attributes.end())
		{
			attributes.erase(known_id_key);
		}

		std::string join_field = g->get_key();
		if (known_id_key != join_field &&
			attributes.find(join_field) == attributes.end())
		{
			attributes.insert(join_field);
		}

		mapnik::grid_renderer<mapnik::grid> ren(*_map,*g,1.0,0,0);
		mapnik::layer const& layer = layers[layerIdx];
		ren.apply(layer,attributes);

	}
}