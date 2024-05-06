import webpack from "webpack";
import path from "path";
import HtmlWebpackPlugin from "html-webpack-plugin";

const SRC_PATH = path.join(__dirname, "src")
const BUILD_PATH = path.join(__dirname, "build")

const node_env = process.env.NODE_ENV == "development" ? "development" : "production"
const current_env = process.env.NODE_ENV === undefined ? "production" : node_env;

const config: webpack.Configuration = {
    entry: "./src/index.tsx",
    mode: current_env,
    output: {
        path: path.resolve(__dirname, 'build'),
        filename: "index.bundle.js",
    },
    resolve: {
        extensions: [".tsx", ".ts", ".js"],
    },
    //devServer: {
    //    static: {
    //      directory: path.join(BUILD_PATH, 'public'),
    //    },
    //},
    devtool: "source-map",
    module: {
        strictExportPresence: true,
        rules: [
            {
                test: /\.(js|jsx|tsx|ts)$/,
                exclude: /node_modules/,
                use: ["babel-loader", "ts-loader"],

            },
            {
                test: /\.(css|scss)$/,
                use: [
                    {
                        loader: "style-loader"
                    },
                    {
                        loader: "css-loader"
                    },
                    {
                        loader: "css-modules-typescript-loader"
                    }
                ]
            },
            {
                test: /\.(png|jpe?g|gif|ttf|otf|jsfl)$/i,
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
        })
    ],
};

export default config;