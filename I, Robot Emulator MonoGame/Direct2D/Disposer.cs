using System;

namespace Direct2D
{
    public static class Disposer
    {
        public static void SafeDispose<T>(ref T? resource) where T : class
        {
            if (resource != null)
            {
                var disposer = resource as IDisposable;
                try
                {
                    disposer?.Dispose();
                }
                catch
                {
                }

                resource = null;
            }
        }
    }
}