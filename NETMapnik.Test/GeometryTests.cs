﻿using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Collections.Generic;
using System.Linq;

namespace NETMapnik.Test
{
    [TestClass]
    public class GeometryTests
    {
        [TestMethod]
        public void Geometry_ToWKT()
        {
            string input = @"{
                type: ""Feature"",
                properties: {},
                geometry: {
                    type: ""Polygon"",
                    coordinates: [[[1,1],[1,2],[2,2],[2,1],[1,1]]]
                }
            }";
            JObject feature = JObject.Parse(input);

            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "csv.input"));
            Dictionary<string, object> options = new Dictionary<string, object>() 
            { 
                { "type","csv"},
                { "inline", "geojson\n'" + feature["geometry"].ToString(Formatting.None) + "'" }
            };
            Datasource ds = new Datasource(options);
            Feature f = ds.Featureset().Next();

            string expected = "POLYGON((1 1,1 2,2 2,2 1,1 1))";
            Assert.AreEqual(expected, f.Geometry().ToWKT());

        }

        [TestMethod]
        public void Geometry_ToWKB()
        {
            string input = @"{
                type: ""Feature"",
                properties: {},
                geometry: {
                    type: ""Polygon"",
                    coordinates: [[[1,1],[1,2],[2,2],[2,1],[1,1]]]
                }
            }";
            JObject feature = JObject.Parse(input);

            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "csv.input"));
            Dictionary<string, object> options = new Dictionary<string, object>() 
            { 
                { "type","csv"},
                { "inline", "geojson\n'" + feature["geometry"].ToString(Formatting.None) + "'" }
            };
            Datasource ds = new Datasource(options);
            Feature f = ds.Featureset().Next();

            string hex = "01030000000100000005000000000000000000f03f000000000000f03f000000000000f03f0000000000000040000000000000004000000000000000400000000000000040000000000000f03f000000000000f03f000000000000f03f";
            byte[] expected = Enumerable.Range(0, hex.Length)
                     .Where(x => x % 2 == 0)
                     .Select(x => Convert.ToByte(hex.Substring(x, 2), 16))
                     .ToArray();
            byte[] actual = f.Geometry().ToWKB();
            CollectionAssert.AreEqual(expected, actual);
        }

        [TestMethod]
        public void Geometry_ToJSON()
        {
            string expected = @"{""type"":""Polygon"",""coordinates"":[[[1,1],[1,2],[2,2],[2,1],[1,1]]]}";
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "csv.input"));
            var options = new Dictionary<string, object>()
            {
                { "type","csv"},
                { "inline", "geojson\n'" + expected + "'" }
            };
            Datasource ds = new Datasource(options);
            Feature f = ds.Featureset().Next();
            string actual = f.Geometry().ToJSON();
            Assert.AreEqual(expected, actual);
        }

        [TestMethod]
        public void Geometry_ToJSON_ProjTransform()
        {
            string expected = @"{""type"":""Polygon"",""coordinates"":[[[1,1],[1,2],[2,2],[2,1],[1,1]]]}";
            Mapnik.RegisterDatasource(Path.Combine(Mapnik.Paths["InputPlugins"], "csv.input"));
            var options = new Dictionary<string, object>()
            {
                { "type","csv"},
                { "inline", "geojson\n'" + expected + "'" }
            };
            Datasource ds = new Datasource(options);
            Feature f = ds.Featureset().Next();
            string actual = f.Geometry().ToJSON();
            Assert.AreEqual(expected, actual);

            Projection src = new Projection("+init=epsg:4326");
            Projection dest = new Projection("+init=epsg:3857");
            ProjTransform tran = new ProjTransform(src, dest);
            string transformed = f.Geometry().ToJSON(tran);
            Assert.AreNotEqual(expected, transformed);
        }
    }
}
