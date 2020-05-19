using SharpDX.Direct2D1;
using System;
using System.Collections.Generic;

namespace Direct2D
{
    public class ResourceCache
    {
        // a dictionary of factories for resources
        readonly Dictionary<string, Func<RenderTarget, object>> Factories = new Dictionary<string, Func<RenderTarget, object>>();

        // dictionary full of resources
        readonly Dictionary<string, object> Resources = new Dictionary<string, object>();

        // the current render target for our resources
        RenderTarget? mRenderTarget = null;

        public RenderTarget? RenderTarget
        {
            get => mRenderTarget;
            set 
            {
                // if the render target is changing
                if (mRenderTarget != value)
                {
                    mRenderTarget = value;

                    // if the new render target is valid
                    if (value != null)
                    {
                        // recreate all resources
                        foreach (var factory in Factories)
                        {
                            object resource = factory.Value(value);

                            // dispose of the old resource
                            if (Resources.TryGetValue(factory.Key, out object? resOld))
                            {
                                if (resOld is IDisposable d)
                                    d.Dispose();
                                Resources.Remove(factory.Key);
                            }

                            // save the new resource
                            Resources.Add(factory.Key, resource);
                        }
                    }
                }
            }
        }

        public int Count
        {
            get { return Resources.Count; }
        }

        public object? this[string key]
        {
            get
            {
                Resources.TryGetValue(key, out object? value);
                return value;
            }
        }

        public Dictionary<string, object>.KeyCollection Keys
        {
            get { return Resources.Keys; }
        }

        public Dictionary<string, object>.ValueCollection Values
        {
            get { return Resources.Values; }
        }

        // - public methods --------------------------------------------------------------

        public void Add(string key, Func<RenderTarget, object> gen)
        {
            object? resOld;
            if (Resources.TryGetValue(key, out resOld))
            {
                Disposer.SafeDispose(ref resOld);
                Factories.Remove(key);
                Resources.Remove(key);
            }

            Factories.Add(key, gen);
            if (mRenderTarget != null)
                Resources.Add(key, gen(mRenderTarget));
        }

        public void Clear()
        {
            foreach (var v in Resources)
            {
                if (v.Value is IDisposable d)
                    d.Dispose();
            }
            Factories.Clear();
            Resources.Clear();
        }

        public bool ContainsKey(string key)
        {
            return Resources.ContainsKey(key);
        }

        public bool ContainsValue(object val)
        {
            return Resources.ContainsValue(val);
        }

        public Dictionary<string, object>.Enumerator GetEnumerator()
        {
            return Resources.GetEnumerator();
        }

        public bool Remove(string key)
        {
            object? res;
            if (Resources.TryGetValue(key, out res))
            {
                Disposer.SafeDispose(ref res);
                Factories.Remove(key);
                Resources.Remove(key);
                return true;
            }
            else
            {
                return false;
            }
        }

        public bool TryGetValue(string key, out object? res)
        {
            return Resources.TryGetValue(key, out res);
        }
    }
}