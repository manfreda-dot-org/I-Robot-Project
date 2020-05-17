using SharpDX.Direct2D1;
using SharpDX.Direct3D;
using SharpDX.Direct3D11;
using SharpDX.DXGI;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Windows;

namespace Direct2D
{
    public abstract class WpfControl : System.Windows.Controls.Image
    {
        #region STATIC
        public static bool IsInDesignMode
        {
            get
            {
                var prop = DesignerProperties.IsInDesignModeProperty;
                var isDesignMode = (bool)DependencyPropertyDescriptor.FromProperty(prop, typeof(FrameworkElement)).Metadata.DefaultValue;
                return isDesignMode;
            }
        }

        private static readonly DependencyPropertyKey AvgRenderTimePropertyKey = DependencyProperty.RegisterReadOnly(
            "AverageRenderTime_ms",
            typeof(double),
            typeof(WpfControl),
            new FrameworkPropertyMetadata(0d, FrameworkPropertyMetadataOptions.None)
            );

        public static readonly DependencyProperty AvgRenderTimeProperty = AvgRenderTimePropertyKey.DependencyProperty;

        public static DependencyProperty RenderWaitProperty = DependencyProperty.Register(
            "RenderWait",
            typeof(int),
            typeof(WpfControl),
            new FrameworkPropertyMetadata(2, OnRenderWaitChanged)
            );
        #endregion

        static readonly double TicksToMs = 1000.0 / Stopwatch.Frequency;

        readonly EventHandler OnRenderingDelegate;

        readonly SharpDX.Direct3D11.Device Device;
        readonly Dx11ImageSource Surface;
        Texture2D? Texture;
        RenderTarget? RenderTarget;
        SharpDX.Direct2D1.Factory? Factory;

        protected readonly ResourceCache ResourceCache = new ResourceCache();

        bool mIsRendering = false;

        readonly Stopwatch RenderTime = new Stopwatch();
        readonly Queue<long> TicksHistory = new Queue<long>();
        long TotalTicks = 0;

        public WpfControl()
        {
            Device = new SharpDX.Direct3D11.Device(DriverType.Hardware, DeviceCreationFlags.BgraSupport);
            Surface = new Dx11ImageSource();

            OnRenderingDelegate = OnRendering;

            if (!WpfControl.IsInDesignMode)
            {
                base.Loaded += new RoutedEventHandler((object sender, RoutedEventArgs e) =>
                {
                    Surface.IsFrontBufferAvailableChanged += OnIsFrontBufferAvailableChanged;

                    CreateAndBindTargets();

                    base.Source = Surface;
                    IsRendering = true;
                });

                base.Unloaded += new RoutedEventHandler((object sender, RoutedEventArgs e) =>
                {
                    IsRendering = false;
                    Surface.IsFrontBufferAvailableChanged -= OnIsFrontBufferAvailableChanged;
                    base.Source = null;

                    Disposer.SafeDispose(ref RenderTarget);
                    Disposer.SafeDispose(ref Factory);
                    Surface.Dispose();
                    Disposer.SafeDispose(ref Texture);
                    Device.Dispose();
                });
            }

            base.Stretch = System.Windows.Media.Stretch.Fill;
        }

        protected abstract void Render(RenderTarget target);

        public double AverageRenderTime_ms
        {
            get { return (double)GetValue(AvgRenderTimeProperty); }
            protected set { SetValue(AvgRenderTimePropertyKey, value); }
        }
        public int RenderWait
        {
            get { return (int)GetValue(RenderWaitProperty); }
            set { SetValue(RenderWaitProperty, value); }
        }

        private void OnRendering(object? sender, EventArgs e)
        {
            RenderTime.Restart();

            if (IsRendering)
            {
                RenderTarget? tgt = RenderTarget;
                if (tgt != null)
                {
                    tgt.BeginDraw();
                    Render(tgt);
                    tgt.EndDraw();
                }

                Device.ImmediateContext.Flush();

                Surface.InvalidateD3DImage();
            }

            UpdateAvgRenderTime(RenderTime.ElapsedTicks);
        }

        private void UpdateAvgRenderTime(long ticks)
        {
            TotalTicks += ticks;
            TicksHistory.Enqueue(ticks);
            if (TicksHistory.Count > 10)
                TotalTicks -= TicksHistory.Dequeue();
            AverageRenderTime_ms = TotalTicks * TicksToMs  / TicksHistory.Count;
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            CreateAndBindTargets();
            base.OnRenderSizeChanged(sizeInfo);
        }

        private void OnIsFrontBufferAvailableChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            IsRendering = Surface.IsFrontBufferAvailable;
        }

        private static void OnRenderWaitChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is WpfControl control)
                control.Surface.RenderWait = (int)e.NewValue;
        }

        private void CreateAndBindTargets()
        {
            Surface.SetRenderTarget(null);

            Disposer.SafeDispose(ref RenderTarget);
            Disposer.SafeDispose(ref Factory);
            Disposer.SafeDispose(ref Texture);

            var width = Math.Max((int)ActualWidth, 100);
            var height = Math.Max((int)ActualHeight, 100);

            var renderDesc = new Texture2DDescription
            {
                BindFlags = BindFlags.RenderTarget | BindFlags.ShaderResource,
                Format = Format.B8G8R8A8_UNorm,
                Width = width,
                Height = height,
                MipLevels = 1,
                SampleDescription = new SampleDescription(1, 0),
                Usage = ResourceUsage.Default,
                OptionFlags = ResourceOptionFlags.Shared,
                CpuAccessFlags = CpuAccessFlags.None,
                ArraySize = 1
            };

            Texture = new Texture2D(Device, renderDesc);

            var surface = Texture.QueryInterface<Surface>();

            Factory = new SharpDX.Direct2D1.Factory();
            var rtp = new RenderTargetProperties(new PixelFormat(Format.Unknown, SharpDX.Direct2D1.AlphaMode.Premultiplied));
            RenderTarget = new RenderTarget(Factory, surface, rtp);
            ResourceCache.RenderTarget = RenderTarget;

            Surface.SetRenderTarget(Texture);

            Device.ImmediateContext.Rasterizer.SetViewport(0, 0, width, height, 0.0f, 1.0f);
        }

        public bool IsRendering
        {
            get => mIsRendering;
            set
            {
                if (mIsRendering != value)
                {
                    mIsRendering = value;

                    if (value)
                        System.Windows.Media.CompositionTarget.Rendering += OnRenderingDelegate;
                    else
                        System.Windows.Media.CompositionTarget.Rendering -= OnRenderingDelegate;
                }
            }
        }
    }
}