const path = require("path");
const HtmlWebpackPlugin = require("html-webpack-plugin");

const BUILD_PATH = path.join(__dirname, "build")
const SRC_PATH = path.join(__dirname, "src")

module.exports = {
  entry: "./src/index.tsx",
  output: {
    path: path.join(__dirname, "build"),
    publicPath: "",
    filename: "index.bundle.js"
  },
  mode: process.env.NODE_ENV || "development",
  resolve: {
    extensions: [".tsx", ".ts", ".js"],
  },
  devServer: { contentBase: BUILD_PATH },
  devtool: 'source-map',
  module: {
    strictExportPresence: true,
    rules: [
      {
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        use: ["babel-loader"],
      },
      {
        test: /\.(ts|tsx)$/,
        exclude: /node_modules/,
        use: ["ts-loader"],
      },
      {
        test: /\.(css|scss)$/,
        use: [
          {
            loader:"style-loader"
          },
          {
            loader:"css-loader"
          },
          {
            loader:"css-modules-typescript-loader"
          }
        ]
      },
      {
        test: /\.(png|jpe?g|gif|ttf|otf)$/i,
        use: [
          {
            loader: 'file-loader',
            options: {
              esModule: false,
              useRelativePath: true,
              name: 'public/[name][hash].[ext]',
            }
          }
        ]
      }
    ],
  },
  plugins: [
    new HtmlWebpackPlugin({
      template: path.join(SRC_PATH, "index.html"),
    }),
  ],
};
